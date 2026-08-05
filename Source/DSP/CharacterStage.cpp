#include "CharacterStage.h"
#include <cmath>

namespace
{
    constexpr double driftRetargetSeconds = 0.6;
    constexpr double gainWobbleRetargetSeconds = 0.9;
    constexpr float maxDriftMs = 0.15f;       // "tens of microseconds" of jitter, capped small even at Drift=100
    constexpr float maxGainWobbleDb = 0.1f;   // "gentle gain fluctuation (+-0.1dB)" per the technical notes

    // output = tanh(driveGain*x) / tanh(driveGain). driveGain stays near-transparent at
    // Saturation=0 the same way (and for the same reason) Gatecrasher's own SlamSaturation had to
    // be fixed to - see that class's history for why a driveGain floor near 1.0 would boost quieter
    // material instead of passing it through cleanly.
    // Both ranges were doubled after listening against the real JN-80: what used to be the maximum
    // turned out to be comfortably usable rather than an extreme, so the top of each knob now
    // reaches twice as far and the OLD maximum sits at the 50% mark. The minima are unchanged, so
    // the bottom half of each control behaves exactly as before - only the headroom above it is new.
    //
    // Solved rather than eyeballed: jmap is linear, so for the old top T to land at 50% the new top
    // must be 2T - min. Saturation: 2(1.2) - 0.02 = 2.38. Noise: 2(-55) - (-75) = -35 dB.
    constexpr float minDriveGain = 0.02f;
    constexpr float maxDriveGain = 2.38f;  // 50% == the previous 1.2

    constexpr float minNoiseFloorDb = -75.0f;
    constexpr float maxNoiseFloorDb = -35.0f; // 50% == the previous -55dB
}

void CharacterStage::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    driftSmoothed.reset(sampleRate, 0.3);
    gainWobbleSmoothed.reset(sampleRate, 0.4);
    reset();
}

void CharacterStage::reset()
{
    driftSmoothed.setCurrentAndTargetValue(0.0f);
    gainWobbleSmoothed.setCurrentAndTargetValue(0.0f);
    driftRetargetCounter = 0.0;
    gainWobbleRetargetCounter = 0.0;
}

void CharacterStage::retargetIfDue(double& counterSamples, double retargetSeconds, juce::SmoothedValue<float>& smoothed)
{
    counterSamples -= 1.0;
    if (counterSamples <= 0.0)
    {
        counterSamples = retargetSeconds * sampleRate;
        smoothed.setTargetValue(random.nextFloat() * 2.0f - 1.0f);
    }
}

float CharacterStage::advanceDrift(int numSamples, float driftPercent)
{
    for (int i = 0; i < numSamples; ++i)
        retargetIfDue(driftRetargetCounter, driftRetargetSeconds, driftSmoothed);
    driftSmoothed.skip(numSamples);

    const float driftAmount = juce::jlimit(0.0f, 1.0f, driftPercent * 0.01f);
    return driftSmoothed.getCurrentValue() * driftAmount * maxDriftMs;
}

void CharacterStage::process(juce::AudioBuffer<float>& buffer, float saturationPercent, float noisePercent)
{
    const float saturation01 = juce::jlimit(0.0f, 1.0f, saturationPercent * 0.01f);
    const float noise01 = juce::jlimit(0.0f, 1.0f, noisePercent * 0.01f);

    const float driveGain = juce::jmap(saturation01, minDriveGain, maxDriveGain);
    const float makeupGain = 1.0f / std::tanh(driveGain);

    const float noiseFloorDb = juce::jmap(noise01, minNoiseFloorDb, maxNoiseFloorDb);
    const float noiseGain = juce::Decibels::decibelsToGain(noiseFloorDb);

    const int numSamples = buffer.getNumSamples();
    wobbleGainScratch.resize((size_t) numSamples);
    for (int i = 0; i < numSamples; ++i)
    {
        retargetIfDue(gainWobbleRetargetCounter, gainWobbleRetargetSeconds, gainWobbleSmoothed);
        const float wobbleDb = gainWobbleSmoothed.getNextValue() * maxGainWobbleDb;
        wobbleGainScratch[(size_t) i] = juce::Decibels::decibelsToGain(wobbleDb);
    }

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            const float driven = std::tanh(data[i] * driveGain) * makeupGain;
            const float hiss = noiseGain * (random.nextFloat() * 2.0f - 1.0f);
            data[i] = driven * wobbleGainScratch[(size_t) i] + hiss;
        }
    }
}
