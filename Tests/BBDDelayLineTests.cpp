#include "TestUtils.h"
#include "../Source/DSP/BBDDelayLine.h"
#include <cmath>
#include <vector>

namespace
{
    juce::AudioBuffer<float> generateSine(int numChannels, int numSamples, double sampleRate, float freqHz, float amplitude)
    {
        juce::AudioBuffer<float> buffer(numChannels, numSamples);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
                data[i] = amplitude * (float) std::sin(2.0 * juce::MathConstants<double>::pi * freqHz * (double) i / sampleRate);
        }
        return buffer;
    }

    float rms(const float* data, int numSamples)
    {
        double sum = 0.0;
        for (int i = 0; i < numSamples; ++i)
            sum += (double) data[i] * (double) data[i];
        return (float) std::sqrt(sum / juce::jmax(1, numSamples));
    }
}

class BBDDelayLineTests final : public juce::UnitTest
{
public:
    BBDDelayLineTests() : juce::UnitTest("BBDDelayLine", "DSP") {}

    void runTest() override
    {
        const double sampleRate = 48000.0;
        juce::dsp::ProcessSpec spec{ sampleRate, 512, 2 };

        beginTest("An impulse's energy reads back near the requested delay time");
        {
            BBDDelayLine line;
            line.prepare(spec);

            const float delayMs = 10.0f;
            const int delaySamples = (int) (0.001 * delayMs * sampleRate);
            const int totalSamples = delaySamples + 200;

            std::vector<float> output((size_t) totalSamples);
            for (int i = 0; i < totalSamples; ++i)
            {
                line.pushSample(0, i == 0 ? 1.0f : 0.0f);
                output[(size_t) i] = line.readTap(0, 0, delayMs);
            }

            int peakIndex = 0;
            float peakValue = 0.0f;
            for (int i = 0; i < totalSamples; ++i)
                if (std::abs(output[(size_t) i]) > peakValue)
                {
                    peakValue = std::abs(output[(size_t) i]);
                    peakIndex = i;
                }

            expect(std::abs(peakIndex - delaySamples) <= 4,
                   "The impulse response should peak within a few samples of the requested delay");
        }

        beginTest("Two simultaneous taps at different delays stay independent");
        {
            BBDDelayLine line;
            line.prepare(spec);

            const int totalSamples = 1000;
            auto noise = generatePinkNoise(1, totalSamples, 77);
            std::vector<float> tapA((size_t) totalSamples), tapB((size_t) totalSamples);

            for (int i = 0; i < totalSamples; ++i)
            {
                line.pushSample(0, noise.getSample(0, i));
                tapA[(size_t) i] = line.readTap(0, 0, 6.0f);
                tapB[(size_t) i] = line.readTap(0, 1, 12.0f);
            }

            bool identical = true;
            for (int i = 0; i < totalSamples; ++i)
                if (std::abs(tapA[(size_t) i] - tapB[(size_t) i]) > 1.0e-5f)
                {
                    identical = false;
                    break;
                }

            expect(! identical, "Two taps at different delay times should not produce identical output");
        }

        beginTest("Reconstruction filter attenuates a high-frequency tone relative to a low-frequency tone");
        {
            BBDDelayLine lineHigh, lineLow;
            lineHigh.prepare(spec);
            lineLow.prepare(spec);

            const int numSamples = 4096;
            auto highTone = generateSine(1, numSamples, sampleRate, 15000.0f, 0.8f);
            auto lowTone = generateSine(1, numSamples, sampleRate, 200.0f, 0.8f);

            std::vector<float> highOut((size_t) numSamples), lowOut((size_t) numSamples);
            for (int i = 0; i < numSamples; ++i)
            {
                lineHigh.pushSample(0, highTone.getSample(0, i));
                highOut[(size_t) i] = lineHigh.readTap(0, 0, 8.0f);

                lineLow.pushSample(0, lowTone.getSample(0, i));
                lowOut[(size_t) i] = lineLow.readTap(0, 0, 8.0f);
            }

            const float highRms = rms(highOut.data() + 1000, numSamples - 1000);
            const float lowRms = rms(lowOut.data() + 1000, numSamples - 1000);

            expect(highRms < lowRms * 0.5f,
                   "A 15kHz tone should be substantially attenuated by the ~7kHz reconstruction filter "
                   "relative to a 200Hz tone");
        }

        beginTest("Output stays finite and bounded on noise across a modulated delay time");
        {
            BBDDelayLine line;
            line.prepare(spec);

            const int numSamples = 4096;
            auto noise = generatePinkNoise(1, numSamples, 909);

            for (int i = 0; i < numSamples; ++i)
            {
                line.pushSample(0, noise.getSample(0, i));
                const float delayMs = 8.0f + 2.0f * std::sin((float) i * 0.001f);
                const float out = line.readTap(0, 0, delayMs);
                expect(std::isfinite(out), "Output must stay finite under a modulated delay time");
                expect(std::abs(out) < 5.0f, "Output should stay in a sane bounded range");
            }
        }
    }
};

static BBDDelayLineTests bbdDelayLineTests;
