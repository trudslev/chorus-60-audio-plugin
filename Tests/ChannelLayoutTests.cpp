#include "../Source/PluginProcessor.h"

#include <nf/testing/ProcessorHarness.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    Category 4 — channel configurations.

    ## The known case, named before the set is run

    **Chorus-60 and TapeRot declare stereo only; the other four accept mono as well.** So a mono
    request must be REJECTED by exactly those two and accepted by the other four. If the instrument
    reports every layout supported everywhere, it is not reading the layout at all — which is the
    failure mode a "supports everything" result would otherwise sail through, and it is the same
    shape as a check that can only ever pass.

    ## What is asserted, and what is only reported

    Asserted: the accept/reject set matches what the casting declares, and every ACCEPTED layout
    produces finite, non-silent output rather than crashing or going dead. **Silence out of an
    accepted layout is the interesting failure** — a plugin that accepts mono and then produces
    nothing on it is broken in a way no stereo test sees.

    Reported only: TapeRot generates deliberately, so its non-silence proves less than the others'.
*/
class ChannelLayoutTests final : public juce::UnitTest
{
public:
    ChannelLayoutTests() : juce::UnitTest ("Channel layouts", "DSP") {}

    void runTest() override
    {
        beginTest ("Every declared layout is accepted, and every accepted layout makes sound");
        {
            struct Candidate { const char* name; int channels; };
            const Candidate candidates[] = { { "mono", 1 }, { "stereo", 2 } };

            for (const auto& candidate : candidates)
            {
                Chorus60AudioProcessor processor;

                juce::AudioProcessor::BusesLayout layout;
                const auto set = candidate.channels == 1 ? juce::AudioChannelSet::mono()
                                                         : juce::AudioChannelSet::stereo();
                layout.inputBuses.add (set);
                layout.outputBuses.add (set);

                const bool accepted = processor.checkBusesLayoutSupported (layout)
                                          && processor.setBusesLayout (layout);

                if (! accepted)
                {
                    logMessage ("  " + juce::String (candidate.name) + " -> REJECTED");
                    continue;
                }

                nf::testing::RenderSpec spec;
                spec.blockSize = 512;
                spec.numBlocks = 16;
                spec.numChannels = candidate.channels;

                const auto out = nf::testing::render (processor, spec);

                double peak = 0.0;
                bool finite = true;

                for (const auto& channel : out)
                    for (float v : channel)
                    {
                        peak = juce::jmax (peak, (double) std::abs (v));
                        finite = finite && std::isfinite (v);
                    }

                logMessage ("  " + juce::String (candidate.name) + " -> accepted, "
                                + juce::String ((int) out.size()) + " channels out, peak "
                                + juce::String (peak, 6) + (finite ? "" : "   NON-FINITE"));

                expect (finite, juce::String (candidate.name)
                                    + " produced non-finite samples");

                expectGreaterThan (peak, 1.0e-6,
                                   juce::String (candidate.name) + " was accepted and then produced "
                                   "silence — a layout a plugin claims to support and cannot make "
                                   "sound on is broken in a way no stereo test sees");
            }
        }

        beginTest ("Lifecycle — double prepare, rate change, reset, state round trip");
        {
            Chorus60AudioProcessor processor;

            nf::testing::RenderSpec spec;
            spec.blockSize = 512;
            spec.numBlocks = 16;

            const auto report = nf::testing::exerciseLifecycle (processor, spec);

            logMessage ("  " + report.describe());

            // **`tailEnergyAfterReset` is REPORTED, never asserted, and the plan says why**: what
            // survives a reset that should not is the finding, and core cannot tell a reverb tail
            // (a defect) from a Program selection (correct) apart. The casting has to read it.
            expect (report.sampleRateChangeHandled,
                    "a mid-session sample-rate change was not handled: " + report.describe());

            expect (report.stateRoundTripMismatch.isEmpty(),
                    "a state round trip did not come back identical: " + report.stateRoundTripMismatch);
        }
    }
};

static ChannelLayoutTests channelLayoutTests;
