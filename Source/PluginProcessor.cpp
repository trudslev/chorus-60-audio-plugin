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
    rate1Param = apvts.getRawParameterValue(ParamIDs::rate1);
    depth1Param = apvts.getRawParameterValue(ParamIDs::depth1);
    rate2Param = apvts.getRawParameterValue(ParamIDs::rate2);
    depth2Param = apvts.getRawParameterValue(ParamIDs::depth2);
    delayCenterParam = apvts.getRawParameterValue(ParamIDs::delayCenter);
    decorrelationParam = apvts.getRawParameterValue(ParamIDs::decorrelation);
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

    modulationEngine1.prepare(sampleRate);
    modulationEngine2.prepare(sampleRate);
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

    const bool engine1On = engine1Param->load() > 0.5f;
    const bool engine2On = engine2Param->load() > 0.5f;
    const float rate1 = rate1Param->load();
    const float depth1 = depth1Param->load();
    const float rate2 = rate2Param->load();
    const float depth2 = depth2Param->load();
    const float delayCenterMs = delayCenterParam->load();

    mainIO.clear(); // now used as the wet accumulator - dry was already captured above

    // CRITICAL: the write into the BBD buffer and the tap reads must be interleaved per sample, not
    // done as two passes over the block. BBDDelayLine::readTap resolves its read position relative
    // to that channel's writeIndex, and only pushSample advances it - so pushing the whole block
    // first leaves writeIndex parked at the block's END while every read happens against it. Each
    // sample in the block then reads from very nearly the same buffer position, and the wet signal
    // collapses to a single value held for the whole block: a staircase stepping at the block rate
    // (~86Hz at 512 samples / 44.1kHz) carrying almost none of the input. Audibly that is a low
    // rumble that survives at Mix 100% while the actual chorus does not.
    float lastOffset1 = 0.0f, lastOffset2 = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        // LFO phase always advances, engaged or not, so re-engaging an engine never phase-jumps.
        const float offset1 = modulationEngine1.getNextOffsetMs(rate1, depth1);
        const float offset2 = modulationEngine2.getNextOffsetMs(rate2, depth2);
        lastOffset1 = offset1;
        lastOffset2 = offset2;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            // This sample goes in before the taps for this sample are read, so both engines read a
            // buffer whose write head is exactly here.
            bbdDelayLine.pushSample(ch, dryBuffer.getSample(ch, i));

            float sample = 0.0f;
            if (engine1On)
                sample += bbdDelayLine.readTap(ch, 0, delayCenterMs + offset1 + driftOffsetMs);
            if (engine2On)
                sample += bbdDelayLine.readTap(ch, 1, delayCenterMs + offset2 + driftOffsetMs);
            mainIO.setSample(ch, i, sample);
        }
    }

    modulationDisplayValue.store((engine1On ? lastOffset1 : 0.0f) + (engine2On ? lastOffset2 : 0.0f),
                                  std::memory_order_relaxed);
    driftDisplayValue.store(driftOffsetMs, std::memory_order_relaxed);

    stereoDecorrelationStage.process(mainIO, decorrelationParam->load());
    characterStage.process(mainIO, saturationParam->load(), noiseParam->load());
    outputMixStage.process(mainIO, dryBuffer, mixParam->load(), trimParam->load());

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
