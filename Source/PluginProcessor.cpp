#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <nf/BlockChunking.h>
#include <cmath>

Chorus60AudioProcessor::Chorus60AudioProcessor()
    : AudioProcessor(BusesProperties()
                          .withInput("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createChorus60ParameterLayout()),
      programManager(apvts)
{
    engine1Param = apvts.getRawParameterValue(ParamIDs::engine1);
    engine2Param = apvts.getRawParameterValue(ParamIDs::engine2);

    const auto bindConfiguration = [this] (ConfigurationParams& target,
                                           const char* rateId,
                                           const char* depthId,
                                           const char* centreId,
                                           const char* decorrId,
                                           const char* imageId)
    {
        target.rate = apvts.getRawParameterValue(rateId);
        target.depth = apvts.getRawParameterValue(depthId);
        target.centre = apvts.getRawParameterValue(centreId);
        target.decorrelation = apvts.getRawParameterValue(decorrId);
        target.image = apvts.getRawParameterValue(imageId);
    };

    bindConfiguration(configI, ParamIDs::rate1, ParamIDs::depth1, ParamIDs::center1,
                      ParamIDs::decorr1, ParamIDs::image1);
    bindConfiguration(configII, ParamIDs::rate2, ParamIDs::depth2, ParamIDs::center2,
                      ParamIDs::decorr2, ParamIDs::image2);
    bindConfiguration(configBoth, ParamIDs::rateB, ParamIDs::depthB, ParamIDs::centerB,
                      ParamIDs::decorrB, ParamIDs::imageB);

    driftParam = apvts.getRawParameterValue(ParamIDs::drift);
    saturationParam = apvts.getRawParameterValue(ParamIDs::saturation);
    noiseParam = apvts.getRawParameterValue(ParamIDs::noise);
    mixParam = apvts.getRawParameterValue(ParamIDs::mix);
    trimParam = apvts.getRawParameterValue(ParamIDs::trim);

    programManager.onProgramListChanged = [this]
    {
        updateHostDisplay(juce::AudioProcessorListener::ChangeDetails().withProgramChanged(true));
    };

    // Construction is single-threaded with no host/automation attached yet, so applying the
    // default program synchronously here (rather than through ProgramManager's async
    // requestProgramChange path) is safe.
    programManager.initialise();
}

void Chorus60AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec{ sampleRate, (juce::uint32) samplesPerBlock,
                                  (juce::uint32) getMainBusNumOutputChannels() };

    modulationEngine.prepare(sampleRate);
    bbdDelayLine.prepare(spec);
    stereoDecorrelationStage.prepare(spec);
    characterStage.prepare(sampleRate);
    outputMixStage.prepare(spec);

    dryBuffer.setSize((int) spec.numChannels, samplesPerBlock, false, false, true);

    inputMeterDb.store(-100.0f, std::memory_order_relaxed);
    outputMeterDb.store(-100.0f, std::memory_order_relaxed);
}

//==============================================================================
/** A host's reset - a transport locate, a buffer clear - propagated to the DSP.

    **JUCE's base implementation is a no-op, and none of the six castings overrode it**, so until
    stage 1c a host asking every plugin in the session to clear itself was answered by nothing
    anywhere. Measured tails surviving a reset: Gatecrasher 0.679, Chorus-60 0.429, Reflect-84 0.111.

    Routed to the same per-stage `reset()` calls `prepareToPlay` already makes, and deliberately NOT
    to `prepareToPlay` itself: re-preparing would also re-run whatever a prepare re-arms, and this
    suite has a measured example of that being audible.
*/
void Chorus60AudioProcessor::reset()
{
    modulationEngine.reset();
    bbdDelayLine.reset();
    stereoDecorrelationStage.reset();
    characterStage.reset();
    outputMixStage.reset();
}

void Chorus60AudioProcessor::releaseResources()
{
}

bool Chorus60AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

Chorus60AudioProcessor::ActiveConfiguration Chorus60AudioProcessor::resolveActiveConfiguration() const
{
    const bool engine1On = engine1Param->load() > 0.5f;
    const bool engine2On = engine2Param->load() > 0.5f;

    ActiveConfiguration result;
    result.engaged = engine1On || engine2On;

    if (! result.engaged)
    {
        result.which = Configuration::bypassed;
        return result;
    }

    // Both latches engaged selects I+II, which is its own configuration rather than I and II
    // running together - see design/BBD-TECHNICAL-NOTES-ADDENDUM.md. The values it uses are its
    // own: typically far faster and centred on a much narrower delay.
    result.which = (engine1On && engine2On) ? Configuration::both
                 : (engine1On              ? Configuration::one
                                           : Configuration::two);

    const ConfigurationParams& source = (engine1On && engine2On) ? configBoth
                                      : (engine1On              ? configI
                                                                : configII);

    result.rateHz = source.rate->load();
    result.depthPercent = source.depth->load();
    result.centreMs = source.centre->load();
    result.decorrelationPercent = source.decorrelation->load();
    result.mono = source.image->load() > 0.5f;
    return result;
}

void Chorus60AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // **The over-delivery policy.** dryBuffer.setSize grows when a host sends more samples than it
    // declared. Chunking removes it: no span exceeds the prepared size, so the growth path is never
    // reached.
    //
    // **THE BUS QUESTION, AND THIS CASTING IS THE THIRD ANSWER OF THREE.** Gatecrasher moved its
    // getBusBuffer calls inside because it has a sidechain; Fifth Member and Reflect-84 had none to
    // move because they read the buffer directly. **Chorus-60 calls getBusBuffer for its MAIN bus
    // and has no second one** — exactly the case that makes the bus COUNT the wrong thing to reason
    // from. Asking once outside would hand every span the whole block's length: the spans would
    // exist, every assertion would pass, and the chunking would be undone. So it moves inside, per
    // span, on the same reasoning as Gatecrasher's and for a different reason.
    //
    // ScopedNoDenormals stays OUTSIDE — it is scoped, so once per call is correct and cheaper.
    nf::processInChunks(buffer, getBlockSize(), [&](juce::AudioBuffer<float>& span)
    {
    auto mainIO = getBusBuffer(span, true, 0);
    const int numSamples = mainIO.getNumSamples();
    const int numChannels = mainIO.getNumChannels();

    dryBuffer.setSize(numChannels, numSamples, false, false, true);
    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer.copyFrom(ch, 0, mainIO, ch, 0, numSamples);

    // Drift perturbs the actual tap position - it has to move the delay TIME rather than colour the
    // audio afterward, per the missing BBD notes' "tiny delay jitter".
    //
    // **Read PER SAMPLE, inside the loop below, and it used to be read once here.** The per-block
    // form was the fourth member of stage 1a's per-block-stepped family and it is the reason this
    // casting's block-size rows failed: a value applied flat across a block makes the output depend
    // on the host's buffer size. Fixed as part of stage 0.5, which is when this casting could be
    // measured again - the figures are in CharacterStage.h beside the function.
    //
    // Nothing about chunking is involved either way. `nf::processInChunks` never fires unless the
    // host over-delivers past the prepared size, and the retarget counter steps per sample, so N
    // spans summing to numSamples retarget at exactly the samples one call would have.
    const float driftPercent = driftParam->load();

    const auto active = resolveActiveConfiguration();

    mainIO.clear(); // now used as the wet accumulator - dry was already captured above

    // CRITICAL: the write into the BBD buffer and the tap reads must be interleaved per sample, not
    // done as two passes over the block. BBDDelayLine::readTap resolves its read position relative
    // to that channel's writeIndex, and only pushSample advances it - so pushing the whole block
    // first leaves writeIndex parked at the block's END while every read happens against it. Each
    // sample in the block then reads from very nearly the same buffer position, and the wet signal
    // collapses to a single value held for the whole block: a staircase stepping at the block rate
    // (~86Hz at 512 samples / 44.1kHz) carrying almost none of the input. Audibly that is a low
    // rumble that survives at Mix 100% while the actual chorus does not.
    float lastOffset = 0.0f;
    float driftOffsetMs = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        // The LFO advances whether or not anything is engaged, so returning from bypass - or
        // switching pages - never phase-jumps.
        const float offset = modulationEngine.getNextOffsetMs(active.rateHz, active.depthPercent);
        lastOffset = offset;

        // Same contract as the LFO above: drift advances whether or not anything is engaged, so its
        // slow wander does not jump when an engine is latched back on.
        driftOffsetMs = characterStage.nextDriftMs(driftPercent);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            // This sample goes in before the tap for this sample is read, so the tap reads a buffer
            // whose write head is exactly here.
            bbdDelayLine.pushSample(ch, dryBuffer.getSample(ch, i));

            if (! active.engaged)
                continue;

            // The entire stereo mechanism of the real circuit: one LFO feeding two BBD lines, with
            // the right channel's modulation inverted 180 degrees. Mono applies no inversion, which
            // is what the hardware does in I+II - both lines then track together and the wet signal
            // is identical in both channels.
            const float channelOffset = (! active.mono && ch == 1) ? -offset : offset;

            // Tap index is per channel, so one engine uses a single tap. The second tap exists for
            // BBDDelayLine's own interpolation state and is no longer a second engine's read.
            mainIO.setSample(ch, i,
                             bbdDelayLine.readTap(ch, 0,
                                                  active.centreMs + channelOffset + driftOffsetMs));
        }
    }

    modulationDisplayValue.store(active.engaged ? lastOffset : 0.0f, std::memory_order_relaxed);
    driftDisplayValue.store(driftOffsetMs, std::memory_order_relaxed);

    if (active.engaged)
    {
        stereoDecorrelationStage.process(mainIO, active.mono ? 0.0f : active.decorrelationPercent);
        characterStage.process(mainIO, saturationParam->load(), noiseParam->load());
        outputMixStage.process(mainIO, dryBuffer, mixParam->load(), trimParam->load());
    }
    else
    {
        // Neither latch engaged is the panel's OFF state, which reads BYPASS - SETTINGS RETAINED
        // and powers down MOD ENGINE, CHARACTER and OUTPUT together. So it is a true bypass: the
        // dry signal passes through untouched rather than being mixed with a silent wet path, which
        // would otherwise fade the instrument out as Mix rose.
        for (int ch = 0; ch < numChannels; ++ch)
            mainIO.copyFrom(ch, 0, dryBuffer, ch, 0, numSamples);
    }

    // IN/OUT meters: simple peak-hold-ish ballistics, updated every block (spec calls for ~6Hz
    // display update, which the GUI's own polling timer governs - this just tracks the real level).
    auto peakDb = [&] (const juce::AudioBuffer<float>& b)
    {
        float peak = 0.0f;
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
        {
            const auto* data = b.getReadPointer(ch);
            for (int i = 0; i < b.getNumSamples(); ++i)
                peak = juce::jmax(peak, std::abs(data[i]));
        }
        return juce::Decibels::gainToDecibels(peak, -100.0f);
    };

    const float newInDb = peakDb(dryBuffer);
    const float newOutDb = peakDb(mainIO);
    const float currentIn = inputMeterDb.load(std::memory_order_relaxed);
    const float currentOut = outputMeterDb.load(std::memory_order_relaxed);
    inputMeterDb.store(newInDb > currentIn ? newInDb : currentIn + 0.3f * (newInDb - currentIn), std::memory_order_relaxed);
    outputMeterDb.store(newOutDb > currentOut ? newOutDb : currentOut + 0.3f * (newOutDb - currentOut), std::memory_order_relaxed);
    });
}

juce::AudioProcessorEditor* Chorus60AudioProcessor::createEditor()
{
    return new Chorus60AudioProcessorEditor(*this);
}

void Chorus60AudioProcessor::setCurrentProgram(int index)
{
    if (! juce::isPositiveAndBelow(index, programManager.getNumPrograms()))
        return;

    // The stale-replay guard, disarmed by this call whether or not it is honoured. A replay carries
    // the position we last reported, so a matching index right after a restore is ignored.
    if (userEdits.consumeRestore() && index == getCurrentProgram())
        return;

    programManager.requestProgramChange(ProgramManager::factoryIdAt(index));
}

void Chorus60AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->setAttribute(LegacyMigration::stateSchemaVersionAttribute, LegacyMigration::currentStateSchemaVersion);
    // **The bank, the identifier, and the full parameter state.** The values make the session sound
    // right; the identity only decides what the panel CALLS them.
    const auto id = programManager.getCurrentProgramId();
    xml->setAttribute(LegacyMigration::programBankAttribute, LegacyMigration::bankAttributeValue(id.bank));
    xml->setAttribute(LegacyMigration::programIdAttribute, id.id);
    xml->setAttribute(LegacyMigration::programNameAttribute, id.displayName);
    copyXmlToBinary(*xml, destData);
}

void Chorus60AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes)); xml != nullptr)
        if (xml->hasTagName(apvts.state.getType()))
        {
            // The schema version was being written but never read, which made it decorative. A v1
            // state restored into the v2 layout produced a silent hybrid: rate1/depth1 survived
            // because those IDs still exist, while center1/decorr1 - new in v2 - fell back to their
            // parameter defaults, so the plugin came up showing neither the saved values nor the
            // program's. Nothing reported a problem; the numbers were simply wrong.
            //
            // v1 predates the paged-engine rework, where delayCenter/decorrelation were single
            // globals and rate1/rate2 spanned 0.2-2Hz rather than 0.05-8Hz. A stored *normalised*
            // value therefore means something different in each, so there is nothing to salvage
            // by reading it - the honest move is to discard it and load the default program.
            // **Two branches, both pinned to literals, because they are different situations.**
            // Too old: v1's normalised values mean something else entirely, so there is nothing to
            // salvage. Too new: written by a later build, and reading it with today's assumptions
            // would produce plausible wrong values rather than an obvious fallback - refusing is the
            // honest answer, and the user needs a newer CHORUS-60 rather than a repair.
            //
            // This replaced `savedSchema != currentStateSchemaVersion`, which was correct exactly
            // once: every bump then discarded the previous version's sessions wholesale, including
            // ones whose parameters had not changed meaning at all.
            const int savedSchema = xml->getIntAttribute(LegacyMigration::stateSchemaVersionAttribute, 1);

            if (LegacyMigration::classifySchema(savedSchema) != LegacyMigration::SchemaVerdict::readable)
            {
                programManager.cancelPendingChange();
                programManager.requestProgramChange(
                    ProgramManager::factoryIdAt(defaultFactoryProgramIndex));
                return;
            }

            programManager.cancelPendingChange();
            apvts.replaceState(juce::ValueTree::fromXml(*xml));

            ProgramId restored;

            if (savedSchema >= LegacyMigration::identitySchemaVersion)
            {
                restored = programManager.resolve(
                    LegacyMigration::bankFromAttribute(
                        xml->getStringAttribute(LegacyMigration::programBankAttribute)),
                    xml->getStringAttribute(LegacyMigration::programIdAttribute),
                    xml->getStringAttribute(LegacyMigration::programNameAttribute));
            }
            else
            {
                // Older readable sessions stored a position. Map it through the CURRENT bank -
                // correct because nothing has shipped and the bank has not moved.
                const int savedIndex =
                    xml->getIntAttribute("chorus60CurrentProgramIndex", defaultFactoryProgramIndex);

                if (savedIndex == -1)
                    restored = ProgramManager::initId();
                else if (juce::isPositiveAndBelow(savedIndex, kNumFactoryPrograms))
                    restored = ProgramManager::factoryIdAt(savedIndex);
                else
                    restored = ProgramManager::factoryIdAt(defaultFactoryProgramIndex);
            }

            programManager.setCurrentProgramWithoutApplying(restored);

            // **Armed AFTER replaceState**, or the restore's own writes would disarm it.
            userEdits.armRestore();
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Chorus60AudioProcessor();
}
