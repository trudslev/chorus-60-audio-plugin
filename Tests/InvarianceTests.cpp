#include "../Source/PluginProcessor.h"
#include "../Source/Parameters.h"
#include "../Source/DSP/BBDDelayLine.h"
#include "../Source/DSP/CharacterStage.h"
#include "../Source/DSP/ModulationEngine.h"
#include "../Source/DSP/StereoDecorrelationStage.h"

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
/*  ## CLOSED — the second cause was CharacterStage::random, and none of the four candidates

    Kept rather than deleted, because how the candidate list went wrong is the reusable part.

    Four candidates were named before the bisection, on the reasoning that the difference survives
    `prepareToPlay`, starts at sample 0 and is signal-dependent: (1) the BBD line's contents,
    (2) the modulation LFO phase — excluded by reading `ModulationEngine::reset`, (3) a filter
    history, (4) a smoother carrying a value across prepare, in the shape of Reflect-84's pre-delay
    defect. Candidate 4 was called the one to test first.

    **It is none of them, and candidate 4 was excluded by measurement before this bisection ran:**
    the suite-wide smoother grep, re-run classifying each site by what its set writes, found
    `CharacterStage::reset` sets both smoothers to a literal `0.0f`. This casting has zero unguarded
    sites. Candidates 1 and 3 are excluded by the stage bisection below — `BBDDelayLine` and
    `StereoDecorrelationStage` are both sample-exact across `prepare()`.

    The answer was `random`, a member of the same class as candidate 4, sitting four lines above the
    smoothers the candidate named. **The list was built from a mechanism — "what carries a value
    across prepare" — and a PRNG carries a POSITION rather than a value**, so nothing phrased that
    way could reach it. What reached it was reading `reset()` and asking which members it does not
    mention, which is a question with no vocabulary in it.

    Same lesson as the suite's other name-based scan: an earlier exclusion said the seed could not
    explain the premise failure because that check is warmed and single-instance while a seed
    difference needs two instances. True of the seed VALUE, false of the stream POSITION — two
    different defects wearing one member, and only the first had been excluded.
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

            // The warmed comparison is what every driver below depends on: a warmed difference
            // means no invariance result in this file is readable. **Both NOISE settings are now
            // asserted exact, and that is an inversion rather than a relaxation.** The NOISE 100
            // arm used to assert `! cd.sampleExact` — that the generator running made the processor
            // irreproducible — which existed to prove the silent arm was not vacuous. It encoded
            // the DEFECT as the property. Once `CharacterStage::reset` seeds its generator that
            // assertion is backwards, so it is inverted to what should hold and the vacuity it
            // guarded against is closed by the positive control below instead.
            expect (cd.sampleExact,
                    "this processor is not reproducible warmed at NOISE "
                        + juce::String (noisePercent, 0) + "%: " + cd.describe());

            if (! ab.sampleExact)
                logMessage ("  NOTE: a first-run-only difference is itself a finding — an instance's "
                            "first playback differs from every later one. Reported, not asserted.");
            }

            /*  **The positive control, and it is load-bearing now that every row above is exact.**
                Five arms reporting sample-exact is indistinguishable from a comparison that cannot
                report anything else, which is the same shape as a suite that ran and proved
                nothing. So one deliberate difference, and `compareRenders` must see it.

                NOISE is the axis because it is the one this file spent the whole hunt on: at NOISE
                0 the hiss floor is `minNoiseFloorDb` = -75 dB rather than silence, so the two
                renders differ by a small but guaranteed amount. A control that came back exact
                would mean the seed had frozen something it should not have. */
            {
                Chorus60AudioProcessor processor;

                nf::testing::RenderSpec spec;
                spec.blockSize = 512;
                spec.numBlocks = 16;

                setNoise (processor, 0.0f);
                const auto quiet = nf::testing::render (processor, spec);

                setNoise (processor, 100.0f);
                const auto loud = nf::testing::render (processor, spec);

                const auto control = nf::testing::compareRenders (quiet, loud);
                logMessage ("  CONTROL NOISE 0 vs 100 -> " + control.describe());

                expect (! control.sampleExact,
                        "the comparison reported two deliberately different renders as identical, so "
                        "every sample-exact row above is a comparison never shown able to fail");
            }
        }

        beginTest ("BISECT BY STAGE — which stage carries state across prepare()");
        {
            /*  **Bisect by stage before bisecting by construction.** The suite's own record is the
                argument: TapeRot's equivalent hunt spent four construction hypotheses and produced
                four refutations and no exclusions, then one stage bisection partitioned the space
                in a single run. This is that run, and each arm is decisive rather than suggestive —
                a stage given the same input twice, with a `prepare()` between, is either exact or
                it is a carrier.

                It mirrors what the premise check does: `nf::testing::render` calls `prepareToPlay`
                on every invocation, so two renders of one processor ARE two prepares of every
                stage inside it.

                ## The prediction, stated before the run

                | Stage | Predicted | Why |
                |---|---|---|
                | `ModulationEngine` | exact | `reset()` zeroes `phase` and `smoothedLfo`, its only two members |
                | `BBDDelayLine` | exact | `reset()` clears the buffer, both write indices and every filter |
                | `StereoDecorrelationStage` | exact | `reset()` resets its one delay line |
                | `CharacterStage` | **DIFFERS** | `reset()` clears both smoothers and both counters and **omits `random`**, and `prepare` does not seed it either |

                `OutputMixStage` is not in the table and is not a fifth clean row: it holds **no
                state at all** — `prepare` and `reset` are both empty bodies. It is structurally
                unable to carry anything, which is a different claim from having been measured and
                found not to, and the sweep's rule is to say which kind each negative is.

                ## Both directions, which is what makes the clean rows mean anything

                Three arms are predicted exact and one is predicted to fail, in one fixture. A
                fixture that reported DIFFERS everywhere would be measuring itself, and one that
                reported exact everywhere would have been unable to fail. The assertion below is
                written as the property that should hold **after** a fix — all four exact — so this
                block is RED today and its CharacterStage row is the thing that turns it green.
            */
            const juce::dsp::ProcessSpec spec { 48000.0, 512, 2 };
            constexpr int blocks = 64;   // 32768 samples = 0.68 s. Both retarget counters start at
                                         // 0.0, so each fires on the first sample of every prepare.

            juce::AudioBuffer<float> reference (2, 512);
            {
                juce::Random r (4321);
                for (int ch = 0; ch < 2; ++ch)
                    for (int i = 0; i < 512; ++i)
                        reference.setSample (ch, i, r.nextFloat() * 2.0f - 1.0f);
            }

            const auto report = [this] (const juce::String& name, auto&& pass)
            {
                const auto a = pass();
                const auto b = pass();

                double worst = 0.0, peak = 0.0;
                for (size_t i = 0; i < a.size(); ++i)
                {
                    worst = juce::jmax (worst, (double) std::abs (a[i] - b[i]));
                    peak  = juce::jmax (peak,  (double) std::abs (b[i]));
                }

                logMessage ("  " + (name + juce::String::repeatedString (" ", 28)).substring (0, 28)
                                 + (worst == 0.0 ? juce::String ("exact")
                                                 : "DIFFERS by " + juce::String (worst, 9))
                                 + "   (peak " + juce::String (peak, 6) + ")");
                return worst;
            };

            ModulationEngine modulation;
            const auto modulationDiff = report ("ModulationEngine", [&]
            {
                modulation.prepare (spec.sampleRate);
                std::vector<float> out;
                for (int block = 0; block < blocks; ++block)
                {
                    out.clear();
                    for (int i = 0; i < 512; ++i)
                        out.push_back (modulation.getNextOffsetMs (0.6f, 55.0f));
                }
                return out;
            });

            BBDDelayLine bbd;
            const auto bbdDiff = report ("BBDDelayLine", [&]
            {
                bbd.prepare (spec);
                std::vector<float> out;
                for (int block = 0; block < blocks; ++block)
                {
                    out.clear();
                    for (int i = 0; i < 512; ++i)
                    {
                        bbd.pushSample (0, reference.getSample (0, i));
                        bbd.pushSample (1, reference.getSample (1, i));
                        out.push_back (bbd.readTap (0, 0, 8.0f));
                    }
                }
                return out;
            });

            StereoDecorrelationStage decorrelation;
            const auto decorrelationDiff = report ("StereoDecorrelationStage", [&]
            {
                decorrelation.prepare (spec);
                juce::AudioBuffer<float> b (2, 512);
                for (int block = 0; block < blocks; ++block)
                {
                    b.makeCopyOf (reference);
                    decorrelation.process (b, 60.0f);
                }

                std::vector<float> out;
                for (int i = 0; i < 512; ++i)
                    out.push_back (b.getSample (1, i));   // right is the decorrelated channel
                return out;
            });

            /*  CharacterStage twice, because `random` has THREE consumers and they are not the
                same size. Splitting the drift return from the audio return costs one extra pass and
                separates the one that moves a delay TIME from the two that colour a signal — which
                is the decomposition the magnitude question below needs. */
            CharacterStage characterDrift;
            const auto driftDiff = report ("CharacterStage / drift ms", [&]
            {
                characterDrift.prepare (spec.sampleRate);
                juce::AudioBuffer<float> b (2, 512);
                std::vector<float> out;
                for (int block = 0; block < blocks; ++block)
                {
                    b.makeCopyOf (reference);
                    for (int i = 0; i < 512; ++i)
                        out.push_back (characterDrift.nextDriftMs (22.0f));   // 22 % is the default
                    characterDrift.process (b, 30.0f, 0.0f);
                }
                return out;
            });

            CharacterStage characterAudio;
            const auto audioDiff = report ("CharacterStage / audio", [&]
            {
                characterAudio.prepare (spec.sampleRate);
                juce::AudioBuffer<float> b (2, 512);
                for (int block = 0; block < blocks; ++block)
                {
                    b.makeCopyOf (reference);
                    for (int i = 0; i < 512; ++i)
                        characterAudio.nextDriftMs (22.0f);
                    characterAudio.process (b, 30.0f, 0.0f);
                }

                std::vector<float> out;
                for (int i = 0; i < 512; ++i)
                    out.push_back (b.getSample (0, i));
                return out;
            });

            logMessage ("  OutputMixStage              structurally stateless — prepare and reset "
                        "are empty bodies, so it is unable to carry rather than measured not to");

            expect (modulationDiff == 0.0,
                    "ModulationEngine differs across prepare() — this was predicted exact from its "
                    "own reset() body, so either the reading is wrong or the fixture is");
            expect (bbdDiff == 0.0,
                    "BBDDelayLine differs across prepare() — predicted exact from its reset() body");
            expect (decorrelationDiff == 0.0,
                    "StereoDecorrelationStage differs across prepare() — predicted exact");

            expect (driftDiff == 0.0,
                    "CharacterStage::nextDriftMs returns a different value stream on a second "
                    "prepare() of the SAME instance. reset() clears driftSmoothed, gainWobbleSmoothed "
                    "and both retarget counters and omits `random` (CharacterStage.h:29), which "
                    "prepare does not seed either — so render 2 continues the stream render 1 left "
                    "off and retargetIfDue draws different values. This moves a delay TIME, not a "
                    "noise floor.");
            expect (audioDiff == 0.0,
                    "CharacterStage::process produces different audio on a second prepare() of the "
                    "same instance, at NOISE 0. Same cause, reaching the output through the gain "
                    "wobble and the -75 dB hiss floor that NOISE 0 does not silence.");
        }

        beginTest ("MAGNITUDE — does the drift path account for the premise figure?");
        {
            /*  **An unrestored PRNG explains that two renders differ without explaining by how
                much**, and 0.159 / 0.357 / 0.542 is a large number to leave unaccounted. A
                candidate that explains the fact and not the size is a candidate, not an answer.

                DRIFT gates one of the three consumers and nothing else: `nextDriftMs` returns
                `driftSmoothed.getNextValue() * driftPercent * 0.01 * maxDriftMs`, so DRIFT 0
                zeroes the tap perturbation while still drawing from `random` at the same rate. The
                other two consumers — the gain wobble and the hiss — have no parameter that gates
                them at all.

                ## Predicted before the run, from the constants rather than from a previous log

                | DRIFT | Tap displacement | Predicted divergence |
                |---|---|---|
                | 0 % | none | the wobble and the hiss alone: ±0.1 dB is ±1.16 % of the wet, so order 1e-2 |
                | 22 % (default) | 0.033 ms = **1.58 samples** at 48 k | order of the wet signal itself — a displacement of more than a sample decorrelates broadband material |
                | 100 % | 0.15 ms = **7.2 samples** | the same or larger |

                And a floor underneath all three, which is the arithmetic rather than an estimate:
                `minNoiseFloorDb` is **-75 dB**, so NOISE 0 does not silence the hiss — it sets it
                to a gain of 1.78e-4. Two independent draws differ by up to 3.56e-4 before the mix.
                **If the DRIFT 0 row comes back at exactly zero, this fixture is wrong**, because
                the code guarantees a generator is still running there.
            */
            for (float driftPercent : { 0.0f, 22.0f, 100.0f })
            {
                Chorus60AudioProcessor processor;
                setParam (processor, ParamIDs::noise, 0.0f);
                setParam (processor, ParamIDs::drift, driftPercent);

                nf::testing::RenderSpec spec;
                spec.blockSize = 512;
                spec.numBlocks = 16;

                nf::testing::render (processor, spec);   // discarded: spends first-run state

                const auto cd = nf::testing::compareRenders (nf::testing::render (processor, spec),
                                                             nf::testing::render (processor, spec));

                logMessage ("  DRIFT " + juce::String (driftPercent, 0).paddedLeft (' ', 3) + "%  -> "
                                + cd.describe());
            }

            logMessage ("  (reported, not asserted — the premise check above is what asserts "
                        "reproducibility. This block exists to account for the SIZE of its failure.)");
        }

        beginTest ("CharacterStage: two fresh instances, identical input, identical output");
        {
            // **FIXED, and this arm is now the pin.** `CharacterStage::reset` seeds `random` from a
            // literal, so both defects this member carried are closed by one line: the clock seed
            // (which this arm catches) and the unrestored stream position (which the premise check
            // catches). Both directions of the same member, and they needed the same fix.
            //
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
                    for (int i = 0; i < 512; ++i)
                        stage.nextDriftMs (100.0f);
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

            /*  **The sweep must be shown to have swept.** Now that every row here is sample-exact,
                a driver that quietly prepared at one size four times would look identical to a
                processor that is genuinely invariant — the same shape as a suite that ran and proved
                nothing. `InvarianceResult::actualBlockSize` is read back off the processor rather
                than echoed from the loop, exactly so a collapsed sweep is visible; asserting it is
                what makes that live rather than a figure in a log nobody reads. */
            const std::vector<int> requested { 64, 128, 511, 2048 };
            expect (results.size() == requested.size(), "the sweep returned a different number of rows than it was asked for");

            for (size_t i = 0; i < juce::jmin (results.size(), requested.size()); ++i)
                expect (results[i].actualBlockSize == requested[i],
                        "the processor prepared at " + juce::String (results[i].actualBlockSize)
                            + " when the sweep asked for " + juce::String (requested[i])
                            + ", so this row is not the block size it claims to be");

            for (const auto& r : results)
                expect (r.sampleExact,
                        "block-size invariance failed — the same sample stream cut differently "
                        "produced different output: " + r.describe());
        }

        beginTest ("Reproducible across reset() ALONE, with the generator driven");
        {
            /*  **A path nothing in this suite could reach until `nf::testing::renderBlocks` existed.**
                `render` calls `prepareToPlay` on every invocation, so every premise check anywhere —
                including the one at the top of this file — is a *prepare* check by construction.
                Prepare once, then `reset()`, render, `reset()`, render is a different question, and a
                host asks it on every transport locate.

                **Chorus-60 is the casting that can assert it**, because stage 0.5 put the seeding in
                `reset()`. The other four generators in the suite are seeded in `prepare()` only, so
                they continue their streams here; whether that is a defect or the correct contract is
                a ruling those castings' rows are measured for. This one is not waiting on it.

                **NOISE and DRIFT at 100, not at defaults.** A generator inaudible in the arm's
                configuration reports reset-clean whatever `reset()` does — which is exactly how
                Fifth Member's and Elmer's energy-after-reset rows came back 0.000 twice for a
                coincidence rather than a property. Driving both consumers is what makes a pass here
                mean the generator was restored rather than that it was never running. */
            Chorus60AudioProcessor processor;
            setParam (processor, ParamIDs::noise, 100.0f);
            setParam (processor, ParamIDs::drift, 100.0f);

            nf::testing::RenderSpec spec;
            spec.blockSize = 512;
            spec.numBlocks = 16;

            const auto r = nf::testing::reproducibleAcrossReset (processor, spec);
            logMessage ("  " + r.describe());

            expect (r.premiseHeld(),
                    "this processor is not reproducible across prepare, so its reset row means "
                    "nothing: " + r.acrossPrepare.describe());

            expect (r.acrossReset.sampleExact,
                    "reset() did not return this processor to the same state — with NOISE and DRIFT "
                    "driven, two renders separated by nothing but reset() differed. CharacterStage "
                    "seeds all three generators in reset() precisely so this holds: "
                        + r.acrossReset.describe());
        }

        beginTest ("BISECT THE BLOCK DEPENDENCE BY STAGE — neutral, then one stage at a time");
        {
            /*  **Readable for the first time.** These rows existed at baseline as 0.733 / 0.725 /
                0.340 / 0.704 and were measuring the unseeded generator; with the premise check green
                they are block dependence and nothing else. Making the drift per-sample moved two of
                the three a long way and barely touched the third:

                | Arm | per-block drift | per-sample drift |
                |---|---|---|
                | 128  | 0.004488230 | 0.000176609 |
                | 511  | 0.167634130 | **0.165607147** |
                | 2048 | 0.086103246 | 0.028221250 |

                511 barely moving is the evidence that a second contributor dominates, and it is
                also why bisecting by CONSTRUCTION would be the wrong move: TapeRot's equivalent
                spent four hypotheses that way and produced four refutations and no exclusions.

                This partitions instead. Every arm drives the same block-size sweep; each restores
                one stage on top of the one before it, so the first arm that stops being exact names
                the stage rather than narrowing to it.

                **The known case is stated before the run, and it is the first two arms.** Both
                engines off is this casting's true bypass — `processBlock` copies the dry buffer
                straight out — and MIX 0 is dry-only through the mix stage. Neither can be block
                dependent for any reason, so if either is not exactly zero the fixture is measuring
                itself and no later row means anything.

                **And the neutral arm must still produce OUTPUT**, which is the other half of that
                rule: a configuration that silences the plugin reports sample-exact for the trivial
                reason and looks like a result. Peak is logged for every arm.
            */
            struct Arm { const char* name; std::vector<std::pair<const char*, float>> params; };

            // Cumulative: each arm is the one above it plus one stage restored.
            const std::vector<Arm> arms
            {
                { "both engines OFF (bypass)",  { { ParamIDs::engine1, 0.0f }, { ParamIDs::engine2, 0.0f } } },
                { "MIX 0 (dry only)",           { { ParamIDs::engine1, 1.0f }, { ParamIDs::mix, 0.0f } } },
                { "wet, everything neutral",    { { ParamIDs::mix, 100.0f }, { ParamIDs::depth1, 0.0f },
                                                  { ParamIDs::drift, 0.0f }, { ParamIDs::decorr1, 0.0f },
                                                  { ParamIDs::saturation, 0.0f }, { ParamIDs::noise, 0.0f } } },
                { "+ LFO depth",                { { ParamIDs::depth1, 55.0f } } },
                { "+ drift",                    { { ParamIDs::drift, 22.0f } } },
                { "+ decorrelation",            { { ParamIDs::decorr1, 60.0f } } },
                { "+ saturation",               { { ParamIDs::saturation, 30.0f } } },
                { "+ noise",                    { { ParamIDs::noise, 14.0f } } },
            };

            std::vector<std::pair<const char*, float>> applied;

            for (const auto& arm : arms)
            {
                for (const auto& p : arm.params)
                    applied.push_back (p);

                Chorus60AudioProcessor processor;
                for (const auto& p : applied)
                    setParam (processor, p.first, p.second);

                nf::testing::RenderSpec spec;
                spec.blockSize = 512;
                spec.numBlocks = 64;

                const auto results = nf::testing::blockSizeInvariance (processor, spec,
                                                                       { 64, 128, 511, 2048 });

                double worst = 0.0;
                for (size_t i = 1; i < results.size(); ++i)          // [0] is the self-comparison
                    worst = juce::jmax (worst, results[i].maxAbsDifference);

                // The peak this arm actually produces — an arm that silences the plugin reports
                // exact for the trivial reason and looks like a result.
                double peak = 0.0;
                for (const auto& channel : nf::testing::render (processor, spec))
                    for (auto v : channel)
                        peak = juce::jmax (peak, (double) std::abs (v));

                logMessage ("  " + (juce::String (arm.name) + juce::String::repeatedString (" ", 28)).substring (0, 28)
                                 + (worst == 0.0 ? juce::String ("exact")
                                                 : "worst " + juce::String (worst, 9))
                                 + "   (self " + juce::String (results.front().maxAbsDifference, 9)
                                 + ", peak " + juce::String (peak, 6) + ")");
            }

            logMessage ("  (reported, not asserted — the Block size suite below is what asserts the "
                        "property. This block exists to say WHICH stage it is.)");
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
    static void setParam (Chorus60AudioProcessor& p, const juce::String& id, float value)
    {
        if (auto* param = dynamic_cast<juce::RangedAudioParameter*> (p.apvts.getParameter (id)))
            param->setValueNotifyingHost (param->getNormalisableRange().convertTo0to1 (value));
    }

    static void setNoise (Chorus60AudioProcessor& p, float percent)
    {
        setParam (p, ParamIDs::noise, percent);
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
