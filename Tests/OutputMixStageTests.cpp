#include "TestUtils.h"
#include "../Source/DSP/OutputMixStage.h"

class OutputMixStageTests final : public juce::UnitTest
{
public:
    OutputMixStageTests() : juce::UnitTest("OutputMixStage", "DSP") {}

    void runTest() override
    {
        const int numChannels = 2;
        const int blockSize = 16;
        juce::dsp::ProcessSpec spec{ 48000.0, (juce::uint32) blockSize, (juce::uint32) numChannels };

        beginTest("mix = 0% returns the dry signal");
        {
            OutputMixStage stage;
            stage.prepare(spec);

            juce::AudioBuffer<float> wet(numChannels, blockSize), dry(numChannels, blockSize);
            for (int ch = 0; ch < numChannels; ++ch)
                for (int i = 0; i < blockSize; ++i)
                {
                    wet.setSample(ch, i, 1.0f);
                    dry.setSample(ch, i, 0.5f);
                }

            stage.process(wet, dry, 0.0f, 0.0f);

            for (int ch = 0; ch < numChannels; ++ch)
                for (int i = 0; i < blockSize; ++i)
                    expectWithinAbsoluteError(wet.getSample(ch, i), 0.5f, 1.0e-5f);
        }

        beginTest("mix = 100% returns the wet signal");
        {
            OutputMixStage stage;
            stage.prepare(spec);

            juce::AudioBuffer<float> wet(numChannels, blockSize), dry(numChannels, blockSize);
            for (int ch = 0; ch < numChannels; ++ch)
                for (int i = 0; i < blockSize; ++i)
                {
                    wet.setSample(ch, i, 0.7f);
                    dry.setSample(ch, i, 0.2f);
                }

            stage.process(wet, dry, 100.0f, 0.0f);

            for (int ch = 0; ch < numChannels; ++ch)
                for (int i = 0; i < blockSize; ++i)
                    expectWithinAbsoluteError(wet.getSample(ch, i), 0.7f, 1.0e-5f);
        }

        beginTest("mix = 50% averages wet and dry");
        {
            OutputMixStage stage;
            stage.prepare(spec);

            juce::AudioBuffer<float> wet(numChannels, blockSize), dry(numChannels, blockSize);
            for (int ch = 0; ch < numChannels; ++ch)
                for (int i = 0; i < blockSize; ++i)
                {
                    wet.setSample(ch, i, 1.0f);
                    dry.setSample(ch, i, 0.0f);
                }

            stage.process(wet, dry, 50.0f, 0.0f);

            for (int ch = 0; ch < numChannels; ++ch)
                for (int i = 0; i < blockSize; ++i)
                    expectWithinAbsoluteError(wet.getSample(ch, i), 0.5f, 1.0e-5f);
        }

        beginTest("trimDb applies a uniform gain on top of the mix");
        {
            OutputMixStage stage;
            stage.prepare(spec);

            juce::AudioBuffer<float> wet(numChannels, blockSize), dry(numChannels, blockSize);
            for (int ch = 0; ch < numChannels; ++ch)
                for (int i = 0; i < blockSize; ++i)
                {
                    wet.setSample(ch, i, 1.0f);
                    dry.setSample(ch, i, 1.0f);
                }

            stage.process(wet, dry, 100.0f, -6.0f);

            const float expectedGain = juce::Decibels::decibelsToGain(-6.0f);
            for (int ch = 0; ch < numChannels; ++ch)
                for (int i = 0; i < blockSize; ++i)
                    expectWithinAbsoluteError(wet.getSample(ch, i), expectedGain, 1.0e-4f);
        }
    }
};

static OutputMixStageTests outputMixStageTests;
