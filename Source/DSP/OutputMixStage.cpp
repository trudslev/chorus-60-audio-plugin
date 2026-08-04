#include "OutputMixStage.h"

void OutputMixStage::prepare(const juce::dsp::ProcessSpec&) {}
void OutputMixStage::reset() {}

void OutputMixStage::process(juce::AudioBuffer<float>& wetBuffer, const juce::AudioBuffer<float>& dryBuffer,
                              float mixPercent, float trimDb)
{
    const float wetGain = juce::jlimit(0.0f, 1.0f, mixPercent * 0.01f);
    const float dryGain = 1.0f - wetGain;
    const float trimGain = juce::Decibels::decibelsToGain(trimDb);

    const int numChannels = juce::jmin(wetBuffer.getNumChannels(), dryBuffer.getNumChannels());
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* wet = wetBuffer.getWritePointer(ch);
        const auto* dry = dryBuffer.getReadPointer(ch);
        for (int i = 0; i < wetBuffer.getNumSamples(); ++i)
            wet[i] = (wet[i] * wetGain + dry[i] * dryGain) * trimGain;
    }
}
