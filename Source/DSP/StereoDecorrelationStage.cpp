#include "StereoDecorrelationStage.h"

void StereoDecorrelationStage::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;

    const int maxDelaySamples = (int) (0.001 * maxExtraDelayMs * sampleRate) + 4;
    rightDelay.setMaximumDelayInSamples(maxDelaySamples);
    rightDelay.prepare(monoSpec);
    reset();
}

void StereoDecorrelationStage::reset()
{
    rightDelay.reset();
}

void StereoDecorrelationStage::process(juce::AudioBuffer<float>& buffer, float decorrelationPercent)
{
    if (buffer.getNumChannels() < 2)
        return;

    const float amount = juce::jlimit(0.0f, 1.0f, decorrelationPercent * 0.01f);
    rightDelay.setDelay((float) (0.001 * maxExtraDelayMs * amount * sampleRate));

    const float invertBlend = amount * maxInvertBlend;
    auto* right = buffer.getWritePointer(1);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        rightDelay.pushSample(0, right[i]);
        const float delayed = rightDelay.popSample(0);
        right[i] = delayed * (1.0f - invertBlend) + (-delayed) * invertBlend;
    }
}
