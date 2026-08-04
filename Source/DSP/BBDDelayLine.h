#pragma once

#include <array>
#include <juce_dsp/juce_dsp.h>

// Shared per-channel delay buffer modeling the bucket-brigade itself, per BBD-TECHNICAL-NOTES.md's
// frequency-response section: an input pre-filter (band-limiting before "storage") and a fixed
// reconstruction-filter rolloff (~6-8kHz) applied per tap read. No feedback path - real BBD chorus
// is "essentially feed-forward."
//
// Two engines read independently-modulated taps from the same underlying buffer (readTap's
// tapIndex selects which of up to maxTaps reconstruction-filter states to use) - each tap needs
// its own filter memory even though both share identical coefficients and the same buffer.
class BBDDelayLine
{
public:
    static constexpr int maxTaps = 2;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void pushSample(int channel, float sample);
    float readTap(int channel, int tapIndex, float delayMs);

private:
    static constexpr int maxChannels = 2;
    static constexpr float maxDelayMs = 50.0f;
    static constexpr float inputPreFilterHz = 12000.0f;
    static constexpr float reconstructionFilterHz = 7000.0f;

    double sampleRate = 44100.0;
    juce::AudioBuffer<float> buffer;
    std::array<int, maxChannels> writeIndex{};

    std::array<juce::dsp::IIR::Filter<float>, maxChannels> inputFilter;
    std::array<std::array<juce::dsp::IIR::Filter<float>, maxTaps>, maxChannels> reconstructionFilter;
};
