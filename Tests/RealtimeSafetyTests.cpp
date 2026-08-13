#include "../Source/PluginProcessor.h"

#include <nf/testing/ProcessorHarness.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    Category 1 of the suite-wide bug sweep, for CHORUS-60.

    **Core owns the drivers; this file owns what CHORUS-60's answers should be.** That split is why
    `nf::testing` could go into core at all — "does this allocate on the audio thread" is one
    question asked of six plugins, while what counts as correct here is knowledge core must not have.

    ## Every allocation result is reported in TWO figures, even when both are zero

    A warm-up run hides any one-off, not only an over-delivery one. A casting that allocates once on
    its very first block reads identically clean under a warmed probe — and that is a different
    finding from never allocating. The first version of Gatecrasher's probe reported exactly that
    error: "no allocation" from a processor that allocates six times on its first oversized block.

    So both are measured and both are logged, here and in all six castings:

      - **cold** — the first block after `prepareToPlay`, nothing absorbed
      - **steady** — after warm-up, which is the per-block cost a host actually pays

    A clean pair is a result worth recording at full length. "Prepared 512, driven 512, first block
    clean, steady clean" is what stops the next audit re-deriving the suspicion from the same lines.

    ## The lead here

    `PluginProcessor.cpp:121` — `dryBuffer.setSize (numChannels, numSamples, false, false, true)`.
    The final `true` is `avoidReallocating` and the buffer is sized in `prepareToPlay` at `:64`, so
    it grows only when a host delivers more samples than it declared.
*/
class RealtimeSafetyTests final : public juce::UnitTest
{
public:
    RealtimeSafetyTests() : juce::UnitTest ("Real-time safety", "DSP") {}

    void runTest() override
    {
        beginTest ("processBlock allocation — matched block size, cold and steady");
        {
            Chorus60AudioProcessor cold;
            const auto c = nf::testing::probeProcessBlockAllocation (cold, 48000.0, 512, 512, 2, 1, 0);

            Chorus60AudioProcessor steadyProc;
            const auto s = nf::testing::probeProcessBlockAllocation (steadyProc, 48000.0, 512, 512, 2);

            logMessage ("  512/512 cold   -> " + c.describe());
            logMessage ("  512/512 steady -> " + s.describe());

            expect (s.clean(), "steady-state processBlock allocates on every block: " + s.describe());
        }

        beginTest ("processBlock allocation — host over-delivers, cold and steady");
        {
            Chorus60AudioProcessor cold;
            const auto c = nf::testing::probeProcessBlockAllocation (cold, 48000.0, 256, 2048, 2, 1, 0);

            Chorus60AudioProcessor steadyProc;
            const auto s = nf::testing::probeProcessBlockAllocation (steadyProc, 48000.0, 256, 2048, 2);

            logMessage ("  256/2048 cold   -> " + c.describe());
            logMessage ("  256/2048 steady -> " + s.describe());

            // The steady figure is the one that must be clean regardless: a per-block allocation at
            // an over-delivered size is a dropout on every block rather than one. The cold figure is
            // reported for the ruling, not asserted — whether a one-off allocation when a host
            // exceeds its declared maximum is a defect or a documented consequence is not core's
            // call and not this file's.
            expect (s.clean(), "processBlock allocates on EVERY over-delivered block: " + s.describe());
        }
    }
};

static RealtimeSafetyTests realtimeSafetyTests;
