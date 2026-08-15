#include "TestUtils.h"
#include "../Source/DSP/BBDDelayLine.h"
#include "../Source/DSP/CharacterStage.h"
#include "../Source/DSP/ModulationEngine.h"
#include "../Source/DSP/OutputMixStage.h"
#include "../Source/DSP/StereoDecorrelationStage.h"

class CPUCheckTests final : public juce::UnitTest
{
public:
    CPUCheckTests() : juce::UnitTest("CPUCheck", "Performance") {}

    void runTest() override
    {
        const double sampleRate = 48000.0;
        const int blockSize = 64;
        const int numChannels = 2;
        const int numIterations = 5000;
        juce::dsp::ProcessSpec spec{ sampleRate, (juce::uint32) blockSize, (juce::uint32) numChannels };
        const double realTimeBudgetMs = ((double) blockSize / sampleRate) * 1000.0;

        beginTest("Full signal chain (both engines active) stays within real-time budget at 48kHz/64 samples");
        {
            ModulationEngine engine1, engine2;
            BBDDelayLine bbdLine;
            StereoDecorrelationStage decorrelation;
            CharacterStage character;
            OutputMixStage outputMix;

            engine1.prepare(sampleRate);
            engine2.prepare(sampleRate);
            bbdLine.prepare(spec);
            decorrelation.prepare(spec);
            character.prepare(sampleRate);
            outputMix.prepare(spec);

            auto dryBuffer = generatePinkNoise(numChannels, blockSize, 606);
            juce::AudioBuffer<float> wetBuffer(numChannels, blockSize);

            const double start = juce::Time::getMillisecondCounterHiRes();
            for (int iter = 0; iter < numIterations; ++iter)
            {
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    const auto* dry = dryBuffer.getReadPointer(ch);
                    for (int i = 0; i < blockSize; ++i)
                        bbdLine.pushSample(ch, dry[i]);
                }

                for (int i = 0; i < blockSize; ++i)
                {
                    // Inside the loop because drift is per sample now, which is also the cost the
                    // shipping processBlock pays - a per-block call here would under-report it.
                    const float driftMs = character.nextDriftMs(40.0f);
                    const float offset1 = engine1.getNextOffsetMs(0.55f, 25.0f);
                    const float offset2 = engine2.getNextOffsetMs(1.0f, 55.0f);
                    for (int ch = 0; ch < numChannels; ++ch)
                    {
                        const float sample = bbdLine.readTap(ch, 0, 8.0f + offset1 + driftMs)
                                            + bbdLine.readTap(ch, 1, 8.0f + offset2 + driftMs);
                        wetBuffer.setSample(ch, i, sample);
                    }
                }

                decorrelation.process(wetBuffer, 70.0f);
                character.process(wetBuffer, 15.0f, 20.0f);
                outputMix.process(wetBuffer, dryBuffer, 50.0f, 0.0f);
            }
            const double end = juce::Time::getMillisecondCounterHiRes();

            const double avgMsPerBlock = (end - start) / (double) numIterations;
            logMessage("Average block time: " + juce::String(avgMsPerBlock, 4) + " ms (budget: "
                       + juce::String(realTimeBudgetMs, 4) + " ms)");
            logMessage("CPU load: " + juce::String(100.0 * avgMsPerBlock / realTimeBudgetMs, 2) + "%");

            expect(avgMsPerBlock < realTimeBudgetMs,
                   "Full chain with both engines active should stay below the real-time budget");
        }
    }
};

static CPUCheckTests cpuCheckTests;
