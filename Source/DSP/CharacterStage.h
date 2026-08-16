#pragma once

#include <juce_dsp/juce_dsp.h>
#include <array>
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

    // Advances the shared drift generator by ONE sample and returns the delay-time perturbation in
    // ms at that sample - PluginProcessor adds it into the tap position before reading BBDDelayLine.
    //
    // **Per sample, and it used to be per block.** The old `advanceDrift (numSamples, ...)` did
    // `skip (numSamples)` and returned `getCurrentValue()`, which the caller then applied flat
    // across the whole block: the value the block used was where the smoother LANDED, not where it
    // travelled. That is the same construction as TapeRot's genSmoothed and Reflect-84's LfoBank,
    // and it made the output depend on the host's buffer size - measured at 0.004488230 / 0.167634130
    // / 0.086103246 for 128 / 511 / 2048 against a 64 reference, every row first diverging at sample
    // 64, which is exactly where the reference takes its second step and the longer arms have not.
    //
    // The retarget cycle being slow (~0.6 s) is what the old design was argued from and it is not
    // the relevant rate: the SMOOTHER ramps over 0.3 s continuously, and quantising a continuous
    // ramp to the block rate is the defect whatever the retarget interval is.
    float nextDriftMs(float driftPercent);

    // Audio-domain processing applied to the assembled wet signal.
    void process(juce::AudioBuffer<float>& buffer, float saturationPercent, float noisePercent);

private:
    void retargetIfDue(double& counterSamples, double retargetSeconds, juce::SmoothedValue<float>& smoothed,
                       juce::Random& generator);

    // Called from prepare() and NOT from reset() — the ruling is beside the definition.
    void seedGenerators();

    static constexpr int maxChannels = 2;   // isBusesLayoutSupported admits stereo in/out only

    double sampleRate = 44100.0;

    /*  **One generator per consumer, and one per channel for the hiss - not the single shared
        `random` this class held until stage 0.5.**

        That member carried FOUR defects, which is why it is worth naming the shape rather than just
        the fix. Two were about its seed and its position: it was clock-seeded, so two instances were
        different instruments, and neither `prepare` nor `reset` restored it, so a second render of
        one instance continued the first's stream. Both are closed by seeding in **`prepare`** — see
        `seedGenerators()`, and note the restore is deliberately not also in `reset()`.

        The other two are why the generators are SPLIT, and they are block-size defects rather than
        determinism ones. A single stream shared by three consumers couples them:

          - the hiss loop draws `2 * numSamples` values per block in channel-major order, so the
            value landing on a given (channel, sample) depends on the host's buffer size;
          - the drift and wobble retargets draw from the same stream, interleaved with those hiss
            draws, so WHICH value a retarget gets depends on how many hiss draws preceded it - and
            that is a buffer-size question too.

        The second is the one with teeth: a retarget sets a delay-time target, so a different draw
        moves the tap. Measured across a 64 / 128 / 511 / 2048 sweep, restoring drift on top of an
        otherwise neutral wet path took the divergence from 0.000354871 to 0.201515645 - and making
        drift per-sample beforehand had barely moved the 511 row, because the staircase was never
        what that row was measuring.

        Seeds are literals so two instances are the same instrument, which
        `Tests/InvarianceTests.cpp` asserts. They differ per consumer only so the three streams are
        not the same sequence read in three places.
    */
    static constexpr juce::int64 driftSeed  = (juce::int64) 0x9E3779B97F4A7C15LL;
    static constexpr juce::int64 wobbleSeed = (juce::int64) 0xA5A5A5A5A5A5A5A5LL;
    static constexpr juce::int64 hissSeed   = (juce::int64) 0xB5B5B5B5B5B5B5B5LL;

    juce::Random driftRandom { driftSeed };
    juce::Random wobbleRandom { wobbleSeed };
    std::array<juce::Random, maxChannels> hissRandom;

    juce::SmoothedValue<float> driftSmoothed { 0.0f };
    double driftRetargetCounter = 0.0;

    juce::SmoothedValue<float> gainWobbleSmoothed { 0.0f };
    double gainWobbleRetargetCounter = 0.0;

    std::vector<float> wobbleGainScratch;
};
