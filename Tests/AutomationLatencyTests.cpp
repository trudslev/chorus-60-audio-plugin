#include "../Source/PluginProcessor.h"

#include <nf/testing/ProcessorHarness.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    Category 5 — automation and latency.

    ## The declared-latency check, and the known case it is introduced against

    Five of the six castings declare no latency at all. **A declaration of zero is a claim like any
    other**, and the only one of the six that can validate the instrument is TapeRot: it is the sole
    casting that declares any (`setLatencySamples (saturator.getLatencySamples())`), its Saturator
    oversamples, and category 4 independently measured a ~20 ms dead window at the start of every
    render which is that latency showing up in a level profile.

    So TapeRot is the known case, stated before the set is run: **the instrument must report a
    non-zero figure there and it must agree with `getLatencySamples()`.** If it reports zero for
    TapeRot, a zero anywhere else means nothing — which is the failure mode five "no latency" claims
    would otherwise sail through.

    ## What agreement means

    An impulse must emerge at exactly the declared latency. Emerging LATER than declared is undeclared
    delay — a host aligns by the declaration, so the plugin's output arrives late and every other
    track is early against it. Emerging EARLIER is a declaration that over-compensates, which pulls
    the plugin ahead. Both are reported; neither is assumed to be the interesting one.
*/
class AutomationLatencyTests final : public juce::UnitTest
{
public:
    AutomationLatencyTests() : juce::UnitTest ("Automation and latency", "DSP") {}

    void runTest() override
    {
        beginTest ("Declared latency against an impulse");
        {
            Chorus60AudioProcessor processor;

            nf::testing::RenderSpec spec;
            spec.blockSize = 512;
            spec.numBlocks = 32;

            // An impulse, not the default noise: the question is WHERE energy first emerges.
            // **`measureImpulseLatency` cannot be used on a casting that GENERATES**, and TapeRot
            // is one. Its noise bed and hum are above any sensible detection threshold at every
            // sample, so "the first output above threshold" is sample 0 whatever the latency is —
            // the warmed run reported exactly that, 0 against a declared 4, and it would have read
            // as a 4-sample over-declaration.
            //
            // So measure DIFFERENTIALLY: render the impulse, render silence, subtract. Everything
            // the plugin generates on its own is deterministic and seeded, so it cancels exactly,
            // and what remains is the impulse's own response. On a casting that generates nothing
            // the silent render is zero and this reduces to the original measurement.
            //
            // (This belongs in core beside measureImpulseLatency rather than in six copies. It is
            // here because moving it costs a tag move and six repins mid-category; the six copies
            // are generated from one template, so they are identical by construction rather than by
            // discipline. Recorded so it is moved when the harness is next touched.)
            const auto renderWith = [&] (bool withImpulse)
            {
                Chorus60AudioProcessor p;

                nf::testing::RenderSpec warmSpec;
                warmSpec.blockSize = spec.blockSize;
                warmSpec.numBlocks = 8;
                nf::testing::render (p, warmSpec);      // spend any first-run state — see category 3

                auto s = spec;
                s.fillInput = [withImpulse] (juce::AudioBuffer<float>& buffer, int blockIndex)
                {
                    buffer.clear();

                    if (withImpulse && blockIndex == 0)
                        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                            buffer.setSample (ch, 0, 1.0f);
                };

                return nf::testing::render (p, s);
            };

            const auto withImpulse = renderWith (true);
            const auto silent      = renderWith (false);

            int measured = -1;
            double impulsePeak = 0.0;

            for (size_t i = 0; i < withImpulse[0].size() && i < silent[0].size(); ++i)
            {
                const double d = std::abs ((double) withImpulse[0][i] - silent[0][i]);
                impulsePeak = juce::jmax (impulsePeak, d);

                if (measured < 0 && d > 1.0e-4)
                    measured = (int) i;
            }

            Chorus60AudioProcessor reference;
            reference.setRateAndBufferSizeDetails (spec.sampleRate, spec.blockSize);
            reference.prepareToPlay (spec.sampleRate, spec.blockSize);
            const int declared = reference.getLatencySamples();

            logMessage ("  impulse response peak " + juce::String (impulsePeak, 6)
                            + " — if that is 0 the two renders are identical and nothing was measured");

            logMessage ("  declared " + juce::String (declared) + " samples, impulse emerged at "
                            + juce::String (measured)
                            + (measured < 0 ? "  (NOTHING EMERGED)" : ""));

            if (measured >= 0 && declared >= 0)
                logMessage ("  difference -> " + juce::String (measured - declared)
                                + " samples ("
                                + juce::String ((measured - declared) * 1000.0 / spec.sampleRate, 3)
                                + " ms)");

            expect (impulsePeak > 1.0e-4,
                    "the impulse produced no measurable response at all, so the latency figure "
                    "below is not a measurement of anything");

            expect (measured >= 0,
                    "no impulse emerged at all within " + juce::String (spec.blockSize * spec.numBlocks)
                        + " samples, so this casting produced nothing to measure latency from");

            // **A tolerance, and it is deliberately tight.** Latency is an integer contract with the
            // host; a few samples of disagreement is still a few samples of misalignment on every
            // track in the session. The band exists only for a first output sample that is genuinely
            // tiny rather than exactly zero.
            expectWithinAbsoluteError (measured, declared, 8,
                                       "the impulse did not emerge where the declared latency says "
                                       "it would. A host aligns by the declaration, so this is "
                                       "session-wide misalignment, not a local artefact.");
        }
    }
};

static AutomationLatencyTests automationLatencyTests;
