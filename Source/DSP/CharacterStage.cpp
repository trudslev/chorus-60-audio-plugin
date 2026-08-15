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

/*  Restores every member, and the generator is a member.

    **This line is what made every other Chorus-60 measurement in the bug sweep unreadable.** The
    four below were always here and are the site root CLAUDE.md cites as a *correct* smoother guard;
    `random` was not, and neither `prepare` nor `reset` seeded it. So a second render of one
    instance continued the stream the first left off, `retargetIfDue` drew different values, and the
    processor was not reproducible against itself even warmed with NOISE at zero. Its block-size
    rows, its offline-against-real-time row and its reset-energy figure were all measuring that.

    **It is a delay TIME the stream moves, which is why the magnitude was so large.** `advanceDrift`
    feeds the BBD tap position, and at the default DRIFT of 22 % that is 1.58 samples at 48 k -
    enough to decorrelate broadband material completely. Measured: the divergence is 0.000273 at
    DRIFT 0 and 0.177544 at DRIFT 22, a factor of 650. The 0.159-0.542 spread across runs was never
    a range to explain - an uncorrelated drift stream makes each run one draw from a distribution.

    **Seeded in `reset()` rather than in `prepare()`, and that is the suite's ruling rather than a
    preference.** TapeRot's `FailureEngine::reset()` clears six members and omits its `random` in
    exactly this shape, and the ruling recorded for it is to seed the *reset*. `prepare` calls this,
    so the reproducibility the premise check needs follows either way; restoration belongs in the
    function whose whole job is restoration, which is where the next reader looks for it.

    The suite's other four generators - TapeRot's `NoiseSource` and `WowFlutter`, Reflect-84's
    `LfoBank`, Fifth Member's `CharacterEngine` - are seeded in `prepare()` only, so a host `reset()`
    does not rewind them. That is a difference rather than a defect: what a plugin owes a reset is a
    cleared tail, not a rewound hiss. Noted here because stage 1c gave all six an
    `AudioProcessor::reset()` and this is the first place the two conventions meet.
*/
void CharacterStage::reset()
{
    driftRandom = juce::Random (driftSeed);
    wobbleRandom = juce::Random (wobbleSeed);
    for (int ch = 0; ch < maxChannels; ++ch)
        hissRandom[(size_t) ch] = juce::Random (hissSeed + ch);

    driftSmoothed.setCurrentAndTargetValue(0.0f);
    gainWobbleSmoothed.setCurrentAndTargetValue(0.0f);
    driftRetargetCounter = 0.0;
    gainWobbleRetargetCounter = 0.0;
}

void CharacterStage::retargetIfDue(double& counterSamples, double retargetSeconds,
                                   juce::SmoothedValue<float>& smoothed, juce::Random& generator)
{
    counterSamples -= 1.0;
    if (counterSamples <= 0.0)
    {
        counterSamples = retargetSeconds * sampleRate;
        smoothed.setTargetValue(generator.nextFloat() * 2.0f - 1.0f);
    }
}

float CharacterStage::nextDriftMs(float driftPercent)
{
    retargetIfDue(driftRetargetCounter, driftRetargetSeconds, driftSmoothed, driftRandom);

    const float driftAmount = juce::jlimit(0.0f, 1.0f, driftPercent * 0.01f);
    return driftSmoothed.getNextValue() * driftAmount * maxDriftMs;
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
        retargetIfDue(gainWobbleRetargetCounter, gainWobbleRetargetSeconds, gainWobbleSmoothed, wobbleRandom);
        const float wobbleDb = gainWobbleSmoothed.getNextValue() * maxGainWobbleDb;
        wobbleGainScratch[(size_t) i] = juce::Decibels::decibelsToGain(wobbleDb);
    }

    // **Per channel, and that is what makes the hiss buffer-size independent.** This loop is
    // channel-major, so one shared generator would hand a given (channel, sample) whichever value
    // fell at `ch * numSamples + i` in the stream - a position that moves with the host's block
    // size. A generator per channel indexes by `i` alone, which does not.
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        auto& generator = hissRandom[(size_t) (ch % maxChannels)];

        for (int i = 0; i < numSamples; ++i)
        {
            const float driven = std::tanh(data[i] * driveGain) * makeupGain;
            const float hiss = noiseGain * (generator.nextFloat() * 2.0f - 1.0f);
            data[i] = driven * wobbleGainScratch[(size_t) i] + hiss;
        }
    }
}
