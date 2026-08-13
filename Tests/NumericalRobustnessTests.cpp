#include "../Source/PluginProcessor.h"

#include <nf/testing/ProcessorHarness.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    Category 2 of the suite-wide bug sweep, for CHORUS-60.

    ## The PARALLEL JOIN shape — the mask is the dry buffer, so it is removed

    Three shapes make output-only scanning insufficient, and this casting has the second:

      - **Cascade** — depth hides the middle. A subnormal can arrive at a stage rather than
        originate in it, and a later stage can scale it back up. TapeRot, 8 stages deep.
      - **Parallel join** — *this one.* The wet chain is 3 stages deep and its result is summed
        against an untouched `dryBuffer` in `OutputMixStage`. A wet-path subnormal added to an
        ordinary dry sample simply disappears into it, so the output reads clean.
      - **Off-signal-path control state** — no output scan of any depth reaches it, because the
        value modulates rather than sums. Elmer's sidechain filter, subnormal for 478 114 of
        480 000 tail samples. This casting has `ModulationEngine` on that third shape — its LFO state modulates the BBD read position and never reaches the output.

    **The mask is removed rather than worked around.** At Mix 100 % the dry contributes nothing to
    the join, so the output IS the wet path and an ordinary output scan reaches all of it. That is
    simpler and less fragile than reconstructing the chain stage by stage, and it answers the
    question the join was hiding.

    What it does not do is separate the wet stages from each other — a subnormal at stage 2 scaled
    back up by stage 3 would still be invisible. TapeRot's per-stage technique is what that
    would need, and it is warranted there because eight stages of 1.35x gain is a structurally
    different chain from this one. Recorded so the limit is visible rather than assumed away.

    ## What a clean row means

    `processBlock` opens with `ScopedNoDenormals`, so a subnormal intermediate is flushed by the
    hardware and never reaches the output. Clean therefore means "the guard covers every path the
    output can see" — not "no denormals occur".
*/
class NumericalRobustnessTests final : public juce::UnitTest
{
public:
    NumericalRobustnessTests() : juce::UnitTest ("Numerical robustness", "DSP") {}

    void runTest() override
    {
        beginTest ("The wet path with the dry join removed — Mix 100 %");
        {
            Chorus60AudioProcessor processor;

            set (processor, ParamIDs::mix, 1.0f);        // fully wet: the join contributes no dry
            logMessage ("  mix -> " + readBack (processor, ParamIDs::mix));

            nf::testing::RenderSpec spec;
            spec.numBlocks = 32;

            // Thousands of blocks, not tens: a decaying value is normal for its first hundred-odd
            // halvings, so a short tail scans the loud part and reports clean.
            const auto report = nf::testing::scanTail (processor, spec, 4000);
            logMessage ("  wet only -> " + report.describe());

            expectEquals (report.nans, 0, "NaN in the wet path: " + report.describe());
            expectEquals (report.infinities, 0, "Inf in the wet path: " + report.describe());
            expectEquals (report.subnormals, 0,
                          "subnormals reached the output with the dry join removed — "
                          "ScopedNoDenormals is not covering the wet path: " + report.describe());
        }

        beginTest ("Default settings, for the contrast the join would have given");
        {
            Chorus60AudioProcessor processor;

            nf::testing::RenderSpec spec;
            spec.numBlocks = 16;

            const auto report = nf::testing::scanTail (processor, spec, 4000);
            logMessage ("  defaults -> " + report.describe());

            expect (report.clean(), "output not clean at defaults: " + report.describe());
        }
    }

private:
    static void set (Chorus60AudioProcessor& p, const char* id, float normalised)
    {
        if (auto* param = p.apvts.getParameter (id))
            param->setValueNotifyingHost (normalised);
    }

    /** The parameter's own text, not the requested value — a sweep that collapsed shows as repeats
        here rather than as a plausible result. */
    static juce::String readBack (Chorus60AudioProcessor& p, const char* id)
    {
        if (auto* param = p.apvts.getParameter (id))
            return param->getCurrentValueAsText();

        return "<missing>";
    }
};

static NumericalRobustnessTests numericalRobustnessTests;
