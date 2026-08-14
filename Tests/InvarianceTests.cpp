#include "../Source/PluginProcessor.h"
#include "../Source/Parameters.h"
#include "../Source/DSP/CharacterStage.h"

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

        beginTest ("CharacterStage: two fresh instances, identical input, different output");
        {
            // **The earlier conclusion here was wrong and this corrects it.** It read: the unseeded
            // generator is real but insufficient, because silencing NOISE left the difference
            // LARGER — so a second cause must sit behind it.
            //
            // Silencing NOISE never silenced the generator. `CharacterStage` holds ONE
            // default-constructed juce::Random (CharacterStage.h:29), which JUCE seeds from the
            // clock, and it has THREE consumers:
            //
            //   :51  the drift retarget          -> moves the BBD read position, so it changes the
            //                                       delay TIME, not a noise floor
            //   :51  the gain-wobble retarget    -> runs unconditionally, no parameter gates it
            //   :91  the hiss                    -> the only one NOISE scales
            //
            // The NOISE parameter scales the hiss's gain and nothing else, and it does not even
            // stop `random.nextFloat()` being called. So that arm removed one consumer of three and
            // was read as removing the generator.
            //
            // Candidate 4 is dead too, and by measurement rather than argument: the suite-wide
            // smoother grep was re-run classifying each site by whether its set is a real value or
            // `setCurrentAndTargetValue(getTargetValue())`, and CharacterStage::reset sets both
            // driftSmoothed and gainWobbleSmoothed to a literal 0.0f. This casting has ZERO
            // unguarded sites; the suite total falls from fourteen to eleven.
            //
            // ## Why this is at class level
            //
            // Two freshly-constructed CharacterStage instances, prepared identically, given
            // byte-identical input. **Every other member of that class has a deterministic
            // initialiser**, so if the two outputs differ, the clock-seeded Random is the only
            // thing they can differ by. No parameter drive can confound it and no other stage is
            // in the path.
            //
            // KNOWN CASE: TapeRot generates too, from a SEEDED per-channel Random, and category 3
            // measured it reproducible when warmed. Same feature, opposite reproducibility — that
            // is the cross-casting control for what this test claims.
            const juce::dsp::ProcessSpec spec { 48000.0, 512, 2 };

            juce::AudioBuffer<float> reference (2, 512);
            {
                juce::Random r (4321);
                for (int ch = 0; ch < 2; ++ch)
                    for (int i = 0; i < 512; ++i)
                        reference.setSample (ch, i, r.nextFloat() * 2.0f - 1.0f);
            }

            const auto runFresh = [&spec, &reference] (float noisePercent)
            {
                CharacterStage stage;
                stage.prepare (spec.sampleRate);

                juce::AudioBuffer<float> b (2, 512);

                // Several blocks, so the 0.6 s retarget counters have had chances to fire.
                for (int block = 0; block < 60; ++block)
                {
                    b.makeCopyOf (reference);
                    stage.advanceDrift (512, 100.0f);
                    stage.process (b, 50.0f, noisePercent);
                }

                std::vector<float> out;
                for (int i = 0; i < 512; ++i)
                    out.push_back (b.getSample (0, i));

                return out;
            };

            for (float noise : { 0.0f, 100.0f })
            {
                const auto a = runFresh (noise);
                const auto b = runFresh (noise);

                double worst = 0.0, peak = 0.0;
                for (size_t i = 0; i < a.size(); ++i)
                {
                    worst = juce::jmax (worst, (double) std::abs (a[i] - b[i]));
                    peak  = juce::jmax (peak,  (double) std::abs (b[i]));
                }

                logMessage ("  NOISE " + juce::String (noise, 0) + "% -> two fresh instances differ by "
                                + juce::String (worst, 9) + " against peak " + juce::String (peak, 6)
                                + " = " + juce::String (peak > 0.0 ? 20.0 * std::log10 (worst / peak) : -99.0, 1)
                                + " dB");

                expect (worst < 1.0e-9,
                        "two freshly-constructed CharacterStage instances given identical input "
                        "produced different output at NOISE " + juce::String (noise, 0) + "%. Every "
                        "other member of that class has a deterministic initialiser, so the "
                        "clock-seeded juce::Random at CharacterStage.h:29 is the only thing they "
                        "can differ by — and at NOISE 0 the hiss is scaled away, so this is the "
                        "generator reaching the output through drift and gain wobble.");
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
