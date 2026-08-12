#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>

// Bundles the missing BBD notes' "nonlinearities" section into one class: Drift (slow random
// delay-time/rate wander - audible as tiny delay jitter, so it has to perturb the actual tap
// position via advanceDrift(), not just color the audio afterward), Saturation (tiny soft-clip),
// Noise (BBD hiss floor, -70 to -75dB per the notes, never complete silence), and a small gain-
// fluctuation wobble (+-0.1dB per the notes).
class CharacterStage
{
public:
    void prepare(double sampleRate);
    void reset();

    // Advances the shared drift generator by numSamples and returns the current delay-time
    // perturbation in ms - PluginProcessor adds this into both engines' tap-position calculations
    // before calling BBDDelayLine, once per block.
    float advanceDrift(int numSamples, float driftPercent);

    // Audio-domain processing applied to the assembled wet signal.
    void process(juce::AudioBuffer<float>& buffer, float saturationPercent, float noisePercent);

private:
    void retargetIfDue(double& counterSamples, double retargetSeconds, juce::SmoothedValue<float>& smoothed);

    double sampleRate = 44100.0;
    juce::Random random;

    juce::SmoothedValue<float> driftSmoothed { 0.0f };
    double driftRetargetCounter = 0.0;

    juce::SmoothedValue<float> gainWobbleSmoothed { 0.0f };
    double gainWobbleRetargetCounter = 0.0;

    std::vector<float> wobbleGainScratch;
};
