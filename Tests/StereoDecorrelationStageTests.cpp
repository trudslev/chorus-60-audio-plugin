#include "TestUtils.h"
#include "../Source/DSP/StereoDecorrelationStage.h"
#include <cmath>

class StereoDecorrelationStageTests final : public juce::UnitTest
{
public:
    StereoDecorrelationStageTests() : juce::UnitTest("StereoDecorrelationStage", "DSP") {}

    void runTest() override
    {
        const int blockSize = 256;
        juce::dsp::ProcessSpec spec{ 48000.0, (juce::uint32) blockSize, 2 };

        beginTest("decorrelation = 0 leaves the right channel unchanged");
        {
            StereoDecorrelationStage stage;
            stage.prepare(spec);

            juce::AudioBuffer<float> buffer(2, blockSize);
            for (int i = 0; i < blockSize; ++i)
            {
                buffer.setSample(0, i, 0.3f);
                buffer.setSample(1, i, 0.5f + 0.001f * (float) i);
            }
            auto original = buffer;

            stage.process(buffer, 0.0f);

            for (int i = 0; i < blockSize; ++i)
                expectWithinAbsoluteError(buffer.getSample(1, i), original.getSample(1, i), 1.0e-5f);
        }

        beginTest("decorrelation = 100 produces a right channel that genuinely differs from the input");
        {
            StereoDecorrelationStage stage;
            stage.prepare(spec);

            juce::AudioBuffer<float> buffer(2, blockSize);
            for (int i = 0; i < blockSize; ++i)
            {
                buffer.setSample(0, i, 0.0f);
                buffer.setSample(1, i, (i % 2 == 0) ? 0.7f : -0.7f);
            }
            auto original = buffer;

            stage.process(buffer, 100.0f);

            bool anyDifference = false;
            for (int i = 0; i < blockSize; ++i)
                if (std::abs(buffer.getSample(1, i) - original.getSample(1, i)) > 1.0e-4f)
                {
                    anyDifference = true;
                    break;
                }

            expect(anyDifference, "At full decorrelation the right channel should differ measurably from the dry input");
        }

        beginTest("Left channel is never touched");
        {
            StereoDecorrelationStage stage;
            stage.prepare(spec);

            juce::AudioBuffer<float> buffer(2, blockSize);
            for (int i = 0; i < blockSize; ++i)
            {
                buffer.setSample(0, i, 0.42f + 0.001f * (float) i);
                buffer.setSample(1, i, 0.1f);
            }
            auto original = buffer;

            stage.process(buffer, 100.0f);

            for (int i = 0; i < blockSize; ++i)
                expectWithinAbsoluteError(buffer.getSample(0, i), original.getSample(0, i), 1.0e-6f);
        }

        beginTest("A mono buffer is left untouched, not crashed on");
        {
            StereoDecorrelationStage stage;
            stage.prepare(spec);

            juce::AudioBuffer<float> mono(1, blockSize);
            mono.setSample(0, 0, 0.5f);
            stage.process(mono, 80.0f);

            expectWithinAbsoluteError(mono.getSample(0, 0), 0.5f, 1.0e-6f);
        }
    }
};

static StereoDecorrelationStageTests stereoDecorrelationStageTests;
