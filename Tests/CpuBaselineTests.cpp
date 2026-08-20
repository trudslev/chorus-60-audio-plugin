#include "../Source/PluginProcessor.h"
#include <nf/testing/CpuBaseline.h>
#include <juce_audio_processors/juce_audio_processors.h>

/*  Category 7: what this casting costs, against a figure that is a FILE.

    **The baseline is `Tests/cpu-baseline.json`, committed, and a missing one is a REFUSAL.** A check
    that silently does nothing when its input is absent is indistinguishable from a check that
    passed, and this suite has three recorded cases of exactly that.

    **Re-baselining is an explicit, committed act.** Set `NF_WRITE_CPU_BASELINE=1` in the
    environment and the run writes the file instead of asserting against it. That is deliberately
    awkward: a figure must never be quietly moved to make a regression go away, and the provenance
    the file records — machine, OS, config, the core commit every casting was pinned at — is what
    makes *one sitting, all six, one machine state* enforceable after the fact rather than a promise.

    **What this replaces.** `CPUCheckTests.cpp` asserted only *average block time < real-time budget* at a
    single 48 k / 64 configuration — a bar a plugin passes at 99 % of a core, which says nothing
    about how many instances will fit. It is superseded rather than kept.
*/
class CpuBaselineTests final : public juce::UnitTest
{
public:
    CpuBaselineTests() : juce::UnitTest ("CPU baseline", "Performance") {}

    void runTest() override
    {
        const auto baselineFile = juce::File (__FILE__).getParentDirectory()
                                       .getChildFile ("cpu-baseline.json");

        std::vector<nf::testing::CpuCell> measured;

        beginTest ("The matrix, measured");
        {
            for (const auto& [block, rate] : nf::testing::standardMatrix())
            {
                for (const bool editorOpen : { false, true })
                {
                    Chorus60AudioProcessor processor;
                    const auto cell = nf::testing::measureCpu (processor, block, rate, editorOpen);
                    measured.push_back (cell);
                    logMessage ("  " + cell.describe());
                }
            }

            expectGreaterThan ((int) measured.size(), 0, "nothing was measured");
        }

        if (juce::SystemStats::getEnvironmentVariable ("NF_WRITE_CPU_BASELINE", {}) == "1")
        {
            beginTest ("RE-BASELINING — writing, not asserting");

            nf::testing::CpuBaseline out;
            out.provenance.machine = juce::SystemStats::getDeviceDescription()
                                   + ", " + juce::String (juce::SystemStats::getNumCpus()) + " cores";
            out.provenance.os = juce::SystemStats::getOperatingSystemName();
            out.provenance.config = "Release";
            out.provenance.coreCommit = NF_CORE_PIN_COMMIT;
            out.provenance.takenOn = juce::Time::getCurrentTime().formatted ("%Y-%m-%d");
            out.cells = measured;
            out.sessionClosedInstances = 24;
            out.sessionOpenInstances = 4;
            out.sessionCoreFraction = 0.5;

            baselineFile.replaceWithText (out.toJson());
            logMessage ("  wrote " + baselineFile.getFullPathName());

            expect (true, "re-baseline run - nothing is asserted, by design");
            return;
        }

        beginTest ("No cell is more than 10 % slower than its recorded baseline");
        {
            juce::String whyNot;
            const auto baseline = nf::testing::CpuBaseline::read (baselineFile, whyNot);

            expect (baseline.has_value(), whyNot);
            if (! baseline.has_value())
                return;

            logMessage ("  baseline taken " + baseline->provenance.takenOn
                        + " on " + baseline->provenance.machine
                        + ", core " + baseline->provenance.coreCommit);

            for (const auto& c : nf::testing::compareToBaseline (measured, *baseline))
            {
                logMessage ("  " + c.describe());
                expect (c.withinTolerance, c.describe());
            }
        }
    }
};

static CpuBaselineTests cpuBaselineTests;
