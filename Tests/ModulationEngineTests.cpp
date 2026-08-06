#include "TestUtils.h"
#include "../Source/DSP/ModulationEngine.h"
#include <cmath>

class ModulationEngineTests final : public juce::UnitTest
{
public:
    ModulationEngineTests() : juce::UnitTest("ModulationEngine", "DSP") {}

    void runTest() override
    {
        const double sampleRate = 48000.0;

        beginTest("Output stays finite and never exceeds the depth-scaled max excursion (~5ms at 100%)");
        {
            ModulationEngine engine;
            engine.prepare(sampleRate);

            const int numSamples = (int) (sampleRate * 3);
            float maxAbs = 0.0f;
            for (int i = 0; i < numSamples; ++i)
            {
                const float offset = engine.getNextOffsetMs(1.0f, 100.0f);
                expect(std::isfinite(offset), "Offset must stay finite");
                maxAbs = juce::jmax(maxAbs, std::abs(offset));
            }

            expect(maxAbs <= 5.1f, "Excursion should stay close to the ~5ms cap even at full depth");
            expect(maxAbs > 0.5f, "A full-depth, several-cycle run should actually swing a meaningful amount");
        }

        beginTest("Depth = 0 produces a silent (zero) offset regardless of rate");
        {
            ModulationEngine engine;
            engine.prepare(sampleRate);

            const int numSamples = (int) (sampleRate * 2);
            for (int i = 0; i < numSamples; ++i)
                expectWithinAbsoluteError(engine.getNextOffsetMs(1.5f, 0.0f), 0.0f, 1.0e-6f);
        }

        beginTest("Peak timing is measurably asymmetric within the cycle - not a symmetric triangle or sine");
        {
            ModulationEngine engine;
            engine.prepare(sampleRate);

            const float rateHz = 1.0f;
            const int samplesPerCycle = (int) (sampleRate / rateHz);

            // Discard the first cycle so the corner-smoothing filter's startup transient settles.
            for (int i = 0; i < samplesPerCycle; ++i)
                engine.getNextOffsetMs(rateHz, 100.0f);

            float maxValue = -1000.0f;
            int maxIndex = 0;
            for (int i = 0; i < samplesPerCycle; ++i)
            {
                const float v = engine.getNextOffsetMs(rateHz, 100.0f);
                if (v > maxValue)
                {
                    maxValue = v;
                    maxIndex = i;
                }
            }

            const float peakFraction = (float) maxIndex / (float) samplesPerCycle;
            expect(std::abs(peakFraction - 0.5f) > 0.02f,
                   "Peak timing should be measurably off-centre, not landing at the cycle's exact midpoint "
                   "the way a symmetric triangle or sine would");
        }

        beginTest("No single-sample jump is wildly out of proportion (basic smoothness sanity)");
        {
            ModulationEngine engine;
            engine.prepare(sampleRate);

            const int numSamples = (int) (sampleRate * 2);
            float maxDelta = 0.0f;
            float previous = engine.getNextOffsetMs(2.0f, 100.0f);
            for (int i = 1; i < numSamples; ++i)
            {
                const float current = engine.getNextOffsetMs(2.0f, 100.0f);
                maxDelta = juce::jmax(maxDelta, std::abs(current - previous));
                previous = current;
            }

            expect(maxDelta < 0.02f, "No single-sample step should approach a meaningful fraction of the full excursion");
        }

        beginTest("Same prepare() sequence produces identical, deterministic output");
        {
            ModulationEngine engineA, engineB;
            engineA.prepare(sampleRate);
            engineB.prepare(sampleRate);

            for (int i = 0; i < 1000; ++i)
                expectWithinAbsoluteError(engineA.getNextOffsetMs(0.7f, 60.0f),
                                           engineB.getNextOffsetMs(0.7f, 60.0f), 1.0e-6f);
        }

        // The I+II configuration runs at 9.75-14Hz, far above the sub-2Hz rates the corner
        // smoothing was originally tuned against. With a fixed 50ms time constant that filter's
        // corner sits near 3.2Hz, so a 12Hz LFO came out attenuated to roughly a quarter - the
        // fast configuration would have been *quieter* than the slow ones rather than faster,
        // silently undoing the distinction design/BBD-TECHNICAL-NOTES-ADDENDUM.md describes.
        //
        // This measures excursion at a fast rate against a slow one at identical depth. It fails
        // if the smoothing is ever pinned back to a fixed constant.
        beginTest("A fast I+II-style rate keeps its excursion instead of being smoothed away");
        {
            const double sampleRate = 48000.0;

            const auto peakExcursion = [sampleRate] (float rateHz)
            {
                ModulationEngine engine;
                engine.prepare(sampleRate);

                // Settle first: the smoother starts from zero, so the opening cycle understates.
                const int settle = (int) (sampleRate * 3.0 / (double) rateHz);
                for (int i = 0; i < settle; ++i)
                    engine.getNextOffsetMs(rateHz, 100.0f);

                float peak = 0.0f;
                const int window = (int) (sampleRate * 2.0 / (double) rateHz);
                for (int i = 0; i < window; ++i)
                    peak = juce::jmax(peak, std::abs(engine.getNextOffsetMs(rateHz, 100.0f)));
                return peak;
            };

            const float slowPeak = peakExcursion(1.0f);
            const float fastPeak = peakExcursion(12.0f);

            expect(slowPeak > 0.1f, "slow rate produced no modulation at all");
            expect(fastPeak > slowPeak * 0.6f,
                   "12Hz excursion collapsed to " + juce::String(fastPeak, 3) + "ms against "
                       + juce::String(slowPeak, 3) + "ms at 1Hz - the corner smoothing is eating "
                       + "the fast I+II configuration");
        }
    }
};

static ModulationEngineTests modulationEngineTests;
