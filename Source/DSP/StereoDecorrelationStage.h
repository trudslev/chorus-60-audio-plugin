#pragma once

#include <juce_dsp/juce_dsp.h>

// Applies genuine L/R difference to the wet signal - per BBD-TECHNICAL-NOTES.md, the real circuit
// achieves stereo width through decorrelation (differing delay/phase, partial polarity difference
// between channels), not panning or a generic stereo-widener. Left stays the reference channel;
// Right gets a small additional delay offset plus a partial polarity blend, both scaled by
// Decorrelation.
class StereoDecorrelationStage
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    void process(juce::AudioBuffer<float>& buffer, float decorrelationPercent);

private:
    static constexpr float maxExtraDelayMs = 0.4f;
    static constexpr float maxInvertBlend = 0.25f;

    double sampleRate = 44100.0;
    juce::dsp::DelayLine<float> rightDelay { 64 };
};
