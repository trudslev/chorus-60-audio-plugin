#include "../Source/PluginProcessor.h"
#include "../Source/Parameters.h"

#include <nf/testing/ProcessorHarness.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    Category 3 — invariance. Does the same audio come out when only the CONTAINER changes?

    ## Why this file leads with premise checks rather than results

    An invariance failure looks like a pass more readily than anything else in this sweep, and it
    also looks like a failure more readily. Reflect-84 produced both in one run: four block-size
    rows all reporting DIFFERS, and every one of them measuring a first-run-only state rather than
    block dependence — because `blockSizeInvariance` compares each size against the FIRST size, so
    its first row is 64 against 64, and that self-comparison differed too.

    So nothing here is believed until the processor is shown to be reproducible against itself, and
    the comparison is shown able to fail. Both are asserted below rather than assumed.
*/
/*  ## The second cause: what it HAS to be, stated before it is bisected for

    Silencing the unseeded generator leaves the warmed comparison differing by 0.508 — larger than
    with the generator running. That last part is evidence, not noise: the hiss was partially
    MASKING the difference, so whatever remains is signal-dependent rather than a fixed offset.

    Combined with "survives prepareToPlay" and "first divergence at sample 0", the candidates are a
    short list, and naming them first is the point — Reflect-84's equivalent took three wrong
    diagnoses precisely because each was reached for after the measurement rather than before:

      1. The BBD line's own contents. Not cleared on prepare would give sample-0 divergence and
         signal dependence, and is the most direct fit.
      2. The modulation LFO phase. ModulationEngine::prepare calls reset() which zeroes phase and
         smoothedLfo (ModulationEngine.cpp:44-55), so this one is ALREADY ruled out by reading —
         recorded so nobody re-derives it.
      3. A filter's history in CharacterStage or the decorrelation stage.
      4. A smoother carrying a value across prepare. CharacterStage::prepare resets driftSmoothed
         and gainWobbleSmoothed with `reset (sampleRate, seconds)` (CharacterStage.cpp:32-33) —
         which is `setCurrentAndTargetValue (target)`, so each snaps to whatever target it last
         held rather than to a known value. **This is exactly Reflect-84's pre-delay defect**, in a
         casting that has two of them rather than one, and it is named here as a candidate rather
         than arrived at after three wrong turns.

    Candidate 4 is the one to test first: it is the same defect the suite has already found once,
    and the two smoothers are drift and gain wobble — both signal-shaping, which fits the masking.
*/
class InvarianceTests final : public juce::UnitTest
{
public:
    InvarianceTests() : juce::UnitTest ("Invariance", "DSP") {}

    void runTest() override
    {
        beginTest ("PREMISE CHECK — reproducible against itself, cold and warmed");
        {
            // Three renders, no parameter writes. A vs B and C vs D separate the two shapes:
            //
            //   A != B, C == D   ->  FIRST-RUN-ONLY state: something is in its constructed
            //                        condition for the first render and its steady one after.
            //   A != B, C != D   ->  ONGOING carry across prepareToPlay.
            //   both exact       ->  reproducible; every result below means what it claims.
            //
            // Reflect-84 came back first-run-only, and the cause was a smoother that never got a
            // setCurrentAndTargetValue — its pre-delay glided up from zero on the first run only.
            // **This casting generates, and the generator is not seeded.** CharacterStage holds a
            // default-constructed juce::Random (CharacterStage.h:29), which JUCE seeds from the
            // clock, and nothing reseeds it on prepare — so its hiss is a different sequence every
            // render and the processor can never be reproducible with NOISE up. Both arms are
            // reported: with the generator running, and with it at zero, which is the only state in
            // which anything else here is measurable.
            //
            // TapeRot generates too and IS reproducible warmed, because its noise draws from a
            // seeded per-channel Random. Same feature, two castings, opposite reproducibility.
            for (float noisePercent : { 0.0f, 100.0f })
            {
            Chorus60AudioProcessor processor;
            setNoise (processor, noisePercent);
            logMessage ("  --- NOISE " + juce::String (noisePercent, 0) + "% ---");

            nf::testing::RenderSpec spec;
            spec.blockSize = 512;
            spec.numBlocks = 16;

            const auto ab = nf::testing::compareRenders (nf::testing::render (processor, spec),
                                                         nf::testing::render (processor, spec));
            const auto cd = nf::testing::compareRenders (nf::testing::render (processor, spec),
                                                         nf::testing::render (processor, spec));

            logMessage ("  cold   A vs B -> " + ab.describe());
            logMessage ("  warmed C vs D -> " + cd.describe());
            logMessage (juce::String ("  => ") + (ab.sampleExact ? "reproducible from construction"
                                                : cd.sampleExact ? "FIRST-RUN-ONLY state — see the note below"
                                                                 : "ONGOING carry across prepareToPlay"));

            // The warmed comparison is what every driver below depends on. A cold difference is a
            // finding in its own right and is reported rather than asserted, because the drivers
            // warm before measuring; a warmed difference means no invariance result is readable.
            if (noisePercent == 0.0f)
                expect (cd.sampleExact,
                        "this processor is not reproducible even warmed WITH THE GENERATOR OFF, so "
                        "a second cause sits behind the unseeded noise: " + cd.describe());
            else
                expect (! cd.sampleExact,
                        "the generator at 100% produced a reproducible render, so the silent arm "
                        "proved nothing — a comparison never shown able to fail");

            if (! ab.sampleExact)
                logMessage ("  NOTE: a first-run-only difference is itself a finding — an instance's "
                            "first playback differs from every later one. Reported, not asserted.");
            }
        }

        beginTest ("Block size — sample-exact at 64 / 128 / 511 / 2048");
        {
            Chorus60AudioProcessor processor;
            warm (processor);

            nf::testing::RenderSpec spec;
            spec.blockSize = 512;
            spec.numBlocks = 64;

            const auto results = nf::testing::blockSizeInvariance (processor, spec,
                                                                   { 64, 128, 511, 2048 });

            for (const auto& r : results)
                logMessage ("  " + r.describe());

            // 511 is prime and shares no factor with the others, so it catches any assumption that
            // a block divides evenly into an internal chunk — the failure a 64/128/2048 sweep walks
            // past because all three share factors.
            //
            // The first row is the size compared against itself. It passing is what makes the other
            // three readable; it failing means the run measured non-determinism.
            expect (! results.empty() && results.front().sampleExact,
                    "the self-comparison failed, so the other rows measured non-determinism rather "
                    "than block dependence");

            for (const auto& r : results)
                expect (r.sampleExact,
                        "block-size invariance failed — the same sample stream cut differently "
                        "produced different output: " + r.describe());
        }

        beginTest ("Offline against real-time");
        {
            Chorus60AudioProcessor processor;
            warm (processor);

            const auto r = nf::testing::offlineAgainstRealtime (processor, {});

            logMessage ("  " + r.describe());

            // **Confirm setNonRealtime changed something observable**, or a passing comparison is
            // only evidence that the flag was ignored.
            if (! r.nonRealtimeWasHonoured)
                logMessage ("  NOTE: setNonRealtime changed nothing this processor reports, so this "
                            "row is 'no offline path exists' rather than 'the offline path agrees'.");

            expect (r.sampleExact || ! r.comparisonWasMeaningful,
                    "offline differs from real-time. Not a defect on its face — this casting would "
                    "have to intend it: " + r.describe());
        }
    }

private:
    /** One discarded render, so any first-run-only state is spent before a driver measures. */
    static void setNoise (Chorus60AudioProcessor& p, float percent)
    {
        if (auto* param = dynamic_cast<juce::RangedAudioParameter*> (p.apvts.getParameter (ParamIDs::noise)))
            param->setValueNotifyingHost (param->getNormalisableRange().convertTo0to1 (percent));
    }

    /** Silences the unseeded generator as well as spending any first-run state: with it running,
        no invariance driver in this file can measure anything. */
    static void warm (Chorus60AudioProcessor& p)
    {
        setNoise (p, 0.0f);

        nf::testing::RenderSpec spec;
        spec.blockSize = 512;
        spec.numBlocks = 4;
        nf::testing::render (p, spec);
    }
};

static InvarianceTests invarianceTests;
