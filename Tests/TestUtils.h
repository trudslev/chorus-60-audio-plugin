#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

inline juce::AudioBuffer<float> generatePinkNoise(int numChannels, int numSamples, juce::int64 seed)
{
    juce::AudioBuffer<float> buffer(numChannels, numSamples);
    juce::Random random(seed);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, b3 = 0.0f, b4 = 0.0f, b5 = 0.0f, b6 = 0.0f;
        auto* data = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            const float white = random.nextFloat() * 2.0f - 1.0f;
            b0 = 0.99886f * b0 + white * 0.0555179f;
            b1 = 0.99332f * b1 + white * 0.0750759f;
            b2 = 0.96900f * b2 + white * 0.1538520f;
            b3 = 0.86650f * b3 + white * 0.3104856f;
            b4 = 0.55000f * b4 + white * 0.5329522f;
            b5 = -0.7616f * b5 - white * 0.0168980f;
            const float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
            b6 = white * 0.115926f;
            data[i] = pink * 0.11f;
        }
    }

    return buffer;
}
