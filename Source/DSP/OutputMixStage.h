#pragma once

#include <juce_dsp/juce_dsp.h>

// Final dry/wet blend (Mix%) and output trim (dB) stage. wetBuffer is processed in place;
// dryBuffer is the pre-chain tap PluginProcessor captures before the chorus chain runs.
// Deliberately does not fake a "+1.1dB chorus makeup" gain when engines are engaged (a stray note
// in the GUI spec's meter math) - the IN/OUT meters just measure real signal levels, and typical
// program material naturally reads a bit hotter post-chorus from constructive comb-filter summing,
// without this stage needing to inject anything artificial.
class OutputMixStage
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    void process(juce::AudioBuffer<float>& wetBuffer, const juce::AudioBuffer<float>& dryBuffer,
                 float mixPercent, float trimDb);
};
