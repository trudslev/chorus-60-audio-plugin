#include "../Source/PluginProcessor.h"

#include <nf/testing/ProcessorHarness.h>

#include <juce_audio_processors/juce_audio_processors.h>

#include <utility>

//==============================================================================
/**
    The pending-program handshake does no heap work under its lock.

    **VST3 delivers a program change as an automatable parameter, so this path can arrive on the
    AUDIO THREAD.** It used to do two `juce::String` assignments inside `pendingLock`. A String copy
    is a refcount increment and reads as safe; the assignment is the other half, releasing whatever
    the target held first — and a refcount reaching zero calls `free()`.

    **The case was never dropout risk.** Measured at 0.12 us worst case against a 10,667 us block
    budget, it is negligible — because a refcount release happens to be cheap, not because anything
    guarantees the path stays heap-free. The argument is that a path the audio thread can reach does
    heap work at all, and that the next person to add a field to `ProgramId` has no reason to know.

    **Why this test arms the sentinel around these two functions and not around
    `requestProgramChange`.** An allocation sentinel counts allocations and frees; it is not
    lock-aware. The copy and the release still happen — they happen in the CALLER's frame now — so a
    probe around the whole call reports the same totals before and after the change. Only a probe
    around the locked region itself can see the difference, and these functions are that region.
*/
class PendingProgramLockTests final : public juce::UnitTest
{
public:
    PendingProgramLockTests() : juce::UnitTest ("Pending program lock", "DSP") {}

    void runTest() override
    {
        /*  **These figures are asserted only where a non-zero count is OURS.**

            `AllocationSentinel`'s counters include a different population on each platform, so the
            same number means three things. On glibc Linux an incidental allocation inside libc that
            lands in the armed window is counted and is not a defect — measured, and it made these
            rows fail with 3 alloc (64 bytes), then 1 alloc (16 bytes), then pass 20 of 20, which
            reads as a flaky test. On Windows `malloc` is not counted at all.

            So the rows are REPORTED everywhere and asserted where they can carry a verdict, and the
            log says which. Same shape as the CPU bar, which asserts only on the machine that
            recorded its baseline. */
        logMessage ("  " + juce::String (nf::testing::AllocationSentinel::describeCoverage()));
        logMessage (juce::String ("  allocation figures below are ")
                        + (nf::testing::AllocationSentinel::countIsAttributable()
                               ? "ASSERTED" : "REPORTED, not asserted"));

        // **A reported row cannot fail, so the instrument gets its own assertion.** Without this a
        // dead sentinel reads zero everywhere and every row looks clean — which is exactly how the
        // detector behaved before the interposition existed. Same role as `comparisons.size() > 0`
        // in the CPU suite: the assertion that survives when the judgement is switched off.
        expect (nf::testing::sentinelIsLive(),
                "the allocation sentinel counted nothing for a known allocation — every allocation "
                "figure in this suite is vacuous");

        beginTest ("exchangePendingProgram and takePendingProgram touch no heap under the lock");
        {
            Chorus60AudioProcessor processor;
            auto& mgr = processor.getProgramManager();

            // Built OUT here, so the sentinel below sees only what the locked region does. A name
            // long enough that juce::String cannot store it inline, or the whole measurement is of
            // a small-string optimisation rather than of a refcount.
            ProgramId first  { ProgramBank::user, "a-program-identifier-long-enough-to-heap-allocate",
                               "A PROGRAM NAME LONG ENOUGH TO HEAP ALLOCATE" };
            ProgramId second { ProgramBank::user, "a-second-identifier-long-enough-to-heap-allocate",
                               "A SECOND NAME LONG ENOUGH TO HEAP ALLOCATE" };

            // Prime it, so the exchange under test releases a REAL previous program rather than an
            // empty one — an empty juce::String shares a global and frees nothing, which would make
            // this pass for the wrong reason.
            ProgramId discarded = mgr.exchangePendingProgram (first);
            juce::ignoreUnused (discarded);

            int exchangeAllocs = 0, exchangeFrees = 0;
            {
                const nf::testing::AllocationSentinel s;
                ProgramId previous = mgr.exchangePendingProgram (std::move (second));
                exchangeAllocs = s.count();
                exchangeFrees = s.frees();

                // `previous` is deliberately kept alive past the sentinel: its release belongs to
                // the caller's frame, which is the whole point of returning it by value.
                juce::ignoreUnused (previous);
            }

            int takeAllocs = 0, takeFrees = 0;
            {
                ProgramId taken;
                const nf::testing::AllocationSentinel s;
                const bool got = mgr.takePendingProgram (taken);
                takeAllocs = s.count();
                takeFrees = s.frees();
                expect (got, "nothing was pending, so the measurement below is of the early return");
            }

            logMessage ("  exchangePendingProgram -> " + juce::String (exchangeAllocs) + " alloc, "
                            + juce::String (exchangeFrees) + " free");
            logMessage ("  takePendingProgram     -> " + juce::String (takeAllocs) + " alloc, "
                            + juce::String (takeFrees) + " free");

            if (nf::testing::AllocationSentinel::countIsAttributable())
                expectEquals (exchangeAllocs, 0, "the locked exchange allocated");
            if (nf::testing::AllocationSentinel::countIsAttributable())
                expectEquals (exchangeFrees, 0,
                              "the locked exchange FREED. Moving the copy and the release into the "
                              "caller's frame is exactly what this change was for");
            if (nf::testing::AllocationSentinel::countIsAttributable())
                expectEquals (takeAllocs, 0, "the locked take allocated");
            if (nf::testing::AllocationSentinel::countIsAttributable())
                expectEquals (takeFrees, 0, "the locked take FREED");
        }

        beginTest ("Shown able to fail — an assignment in the same place does free");
        {
            /*  Without this, four zeros are indistinguishable from a sentinel that hooks nothing or
                from strings short enough to live inline. The pre-change construction is performed
                directly: assigning over a ProgramId that holds heap-allocated strings must release
                them, and the sentinel must see it. */
            ProgramId held { ProgramBank::user, "a-program-identifier-long-enough-to-heap-allocate",
                             "A PROGRAM NAME LONG ENOUGH TO HEAP ALLOCATE" };
            ProgramId incoming { ProgramBank::user, "a-second-identifier-long-enough-to-heap-allocate",
                                 "A SECOND NAME LONG ENOUGH TO HEAP ALLOCATE" };

            int frees = 0;
            {
                const nf::testing::AllocationSentinel s;
                held = incoming;              // the assignment the locked region used to perform
                frees = s.frees();
            }

            logMessage ("  plain assignment       -> " + juce::String (frees) + " free");

            expectGreaterThan (frees, 0,
                               "**THIS PROBE CANNOT SEE A FREE.** Assigning over a ProgramId holding "
                               "heap-allocated strings released nothing the sentinel could count, so "
                               "the zeros above are not evidence that the locked region is clean");
        }
    }
};

static PendingProgramLockTests pendingProgramLockTests;
