#include "TestUtils.h"
#include "../Source/DSP/CharacterStage.h"
#include <cmath>

class CharacterStageTests final : public juce::UnitTest
{
public:
    CharacterStageTests() : juce::UnitTest("CharacterStage", "DSP") {}

    void runTest() override
    {
        const double sampleRate = 48000.0;

        beginTest("Drift offset stays bounded and finite even at full Drift over a long run");
        {
            CharacterStage stage;
            stage.prepare(sampleRate);

            const int blockSize = 512;
            const int numBlocks = 200;
            float maxAbs = 0.0f;
            for (int b = 0; b < numBlocks; ++b)
            {
                const float offset = stage.advanceDrift(blockSize, 100.0f);
                expect(std::isfinite(offset), "Drift offset must stay finite");
                maxAbs = juce::jmax(maxAbs, std::abs(offset));
            }

            expect(maxAbs <= 0.16f, "Drift should stay within its small capped range even at 100%");
        }

        beginTest("Drift = 0 produces a silent (zero) offset");
        {
            CharacterStage stage;
            stage.prepare(sampleRate);

            for (int b = 0; b < 50; ++b)
                expectWithinAbsoluteError(stage.advanceDrift(512, 0.0f), 0.0f, 1.0e-6f);
        }

        beginTest("Saturation = 0, Noise = 0 stays close to unity gain on a moderate-level sine");
        {
            CharacterStage stage;
            stage.prepare(sampleRate);

            const int numChannels = 2;
            const int blockSize = 512;
            juce::AudioBuffer<float> buffer(numChannels, blockSize);
            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto* data = buffer.getWritePointer(ch);
                for (int i = 0; i < blockSize; ++i)
                    data[i] = 0.3f * std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * (float) i / (float) sampleRate);
            }

            stage.process(buffer, 0.0f, 0.0f);

            float maxAbs = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
                for (int i = 0; i < blockSize; ++i)
                    maxAbs = juce::jmax(maxAbs, std::abs(buffer.getSample(ch, i)));

            expectWithinAbsoluteError(maxAbs, 0.3f, 0.03f);
        }

        beginTest("Higher Noise produces a measurably louder noise floor on a silent input");
        {
            CharacterStage lowNoise, highNoise;
            lowNoise.prepare(sampleRate);
            highNoise.prepare(sampleRate);

            const int numChannels = 2;
            const int blockSize = 4096;
            juce::AudioBuffer<float> bufferLow(numChannels, blockSize), bufferHigh(numChannels, blockSize);
            bufferLow.clear();
            bufferHigh.clear();

            lowNoise.process(bufferLow, 0.0f, 0.0f);
            highNoise.process(bufferHigh, 0.0f, 100.0f);

            auto rms = [] (const juce::AudioBuffer<float>& b)
            {
                double sum = 0.0;
                int count = 0;
                for (int ch = 0; ch < b.getNumChannels(); ++ch)
                    for (int i = 0; i < b.getNumSamples(); ++i)
                    {
                        sum += (double) b.getSample(ch, i) * (double) b.getSample(ch, i);
                        ++count;
                    }
                return (float) std::sqrt(sum / juce::jmax(1, count));
            };

            expect(rms(bufferHigh) > rms(bufferLow) * 2.0f,
                   "Noise=100 should produce a substantially louder noise floor than Noise=0 on silence");
        }

        beginTest("Output stays finite and bounded across the full Saturation/Noise range on real program material");
        {
            CharacterStage stage;
            stage.prepare(sampleRate);

            auto buffer = generatePinkNoise(2, 2048, 55);
            stage.process(buffer, 100.0f, 100.0f);

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    const float v = buffer.getSample(ch, i);
                    expect(std::isfinite(v), "Output must stay finite");
                    expect(std::abs(v) < 5.0f, "Output should stay in a sane bounded range");
                }
        }
    }
};

static CharacterStageTests characterStageTests;
