#include "PluginProcessor.h"
#include "PluginEditor.h"
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
                                           const char* monoId)
    {
        target.rate = apvts.getRawParameterValue(rateId);
        target.depth = apvts.getRawParameterValue(depthId);
        target.centre = apvts.getRawParameterValue(centreId);
        target.decorrelation = apvts.getRawParameterValue(decorrId);
        target.mono = apvts.getRawParameterValue(monoId);
    };

    bindConfiguration(configI, ParamIDs::rate1, ParamIDs::depth1, ParamIDs::center1,
                      ParamIDs::decorr1, ParamIDs::mono1);
    bindConfiguration(configII, ParamIDs::rate2, ParamIDs::depth2, ParamIDs::center2,
                      ParamIDs::decorr2, ParamIDs::mono2);
    bindConfiguration(configBoth, ParamIDs::rateB, ParamIDs::depthB, ParamIDs::centerB,
                      ParamIDs::decorrB, ParamIDs::monoB);

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
    result.mono = source.mono->load() > 0.5f;
    return result;
}

void Chorus60AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto mainIO = getBusBuffer(buffer, true, 0);
    const int numSamples = mainIO.getNumSamples();
    const int numChannels = mainIO.getNumChannels();

    dryBuffer.setSize(numChannels, numSamples, false, false, true);
    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer.copyFrom(ch, 0, mainIO, ch, 0, numSamples);

    // Drift is a single slow-moving value per block (retargets every ~0.6s - see CharacterStage),
    // shared by both engines' tap-position calculations.
    const float driftOffsetMs = characterStage.advanceDrift(numSamples, driftParam->load());

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
    for (int i = 0; i < numSamples; ++i)
    {
        // The LFO advances whether or not anything is engaged, so returning from bypass - or
        // switching pages - never phase-jumps.
        const float offset = modulationEngine.getNextOffsetMs(active.rateHz, active.depthPercent);
        lastOffset = offset;

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
}

juce::AudioProcessorEditor* Chorus60AudioProcessor::createEditor()
{
    return new Chorus60AudioProcessorEditor(*this);
}

void Chorus60AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->setAttribute(LegacyMigration::stateSchemaVersionAttribute, LegacyMigration::currentStateSchemaVersion);
    xml->setAttribute("chorus60CurrentProgramIndex", programManager.getCurrentProgram());
    copyXmlToBinary(*xml, destData);
}

void Chorus60AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes)); xml != nullptr)
        if (xml->hasTagName(apvts.state.getType()))
        {
            programManager.cancelPendingChange();
            apvts.replaceState(juce::ValueTree::fromXml(*xml));

            const int savedProgramIndex =
                xml->getIntAttribute("chorus60CurrentProgramIndex", defaultFactoryProgramIndex);
            programManager.setCurrentProgramIndexWithoutApplying(
                juce::isPositiveAndBelow(savedProgramIndex, programManager.getNumPrograms())
                    ? savedProgramIndex
                    : defaultFactoryProgramIndex);
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Chorus60AudioProcessor();
}
