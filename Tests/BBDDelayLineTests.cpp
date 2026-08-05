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

        // Depth now reaches +-5ms, so at the minimum Delay Center (5ms) a full-depth sweep drives
        // the requested tap position to zero and below. Unclamped that wraps to stale audio a whole
        // buffer old - a discontinuity, not a delay - so readTap clamps. This checks the clamp holds
        // for the values the panel can actually produce, including the negative ones.
        beginTest("Tap position is clamped to a readable window even at or below zero delay");
        {
            BBDDelayLine line;
            line.prepare(spec);

            for (int n = 0; n < 4096; ++n)
            {
                line.pushSample(0, std::sin(juce::MathConstants<float>::twoPi * 300.0f * (float) n
                                             / (float) sampleRate));
                line.readTap(0, 0, 8.0f);
            }

            for (const float requested : { -5.0f, -0.15f, 0.0f, 0.05f, 0.1f, 1.0f, 49.0f, 80.0f })
            {
                for (int n = 0; n < 256; ++n)
                {
                    line.pushSample(0, 0.5f);
                    const float out = line.readTap(0, 0, requested);
                    expect(std::isfinite(out),
                           "Tap must stay finite at a requested delay of " + juce::String(requested) + "ms");
                    expect(std::abs(out) < 5.0f,
                           "Tap must stay bounded at a requested delay of " + juce::String(requested) + "ms");
                }
            }
        }

        // Regression guard for a signal-flow bug in PluginProcessor::processBlock: it pushed the
        // whole block into the line and only then read the taps. readTap resolves its position
        // relative to writeIndex, and only pushSample advances it, so every read in the block
        // happened against a write head already parked at the block's end - the wet output became a
        // single value held for the entire block (a staircase at block rate, audibly a low rumble
        // with essentially none of the input in it). Every existing test above passed straight
        // through that, because a held value is still finite, still bounded, and still "energy near
        // the requested delay".
        //
        // This asserts the property those missed: driven correctly, the tap output has to *vary*
        // within a block, and has to actually track the input.
        beginTest("A tap read interleaved with writes reproduces the signal, not a held value");
        {
            BBDDelayLine line;
            line.prepare(spec);

            const float delayMs = 10.0f;
            const float frequency = 220.0f;
            const int blockSize = 512;

            // Prime the line so the tap is reading real written signal rather than initial silence.
            int n = 0;
            const auto inputAt = [&] (int index)
            {
                return std::sin(juce::MathConstants<float>::twoPi * frequency * (float) index
                                 / (float) sampleRate);
            };

            for (; n < (int) (0.001 * delayMs * sampleRate) + blockSize * 4; ++n)
            {
                line.pushSample(0, inputAt(n));
                line.readTap(0, 0, delayMs);
            }

            // One block, driven the way processBlock must drive it.
            std::vector<float> block;
            block.reserve((size_t) blockSize);
            for (int i = 0; i < blockSize; ++i, ++n)
            {
                line.pushSample(0, inputAt(n));
                block.push_back(line.readTap(0, 0, delayMs));
            }

            // Zero crossings, not peak-to-peak range: a stale read still feeds the reconstruction
            // filter, and an IIR fed one constant settles towards it, which is a big enough
            // excursion to satisfy a naive min/max test. What it cannot do is OSCILLATE. A 220Hz
            // sine spans ~2.35 cycles across 512 samples at 48kHz, so a tap actually carrying the
            // signal crosses zero four or more times; a settling transient crosses at most once.
            int zeroCrossings = 0;
            for (size_t i = 1; i < block.size(); ++i)
                if ((block[i - 1] < 0.0f) != (block[i] < 0.0f))
                    ++zeroCrossings;

            float peak = 0.0f;
            for (const float v : block)
                peak = juce::jmax(peak, std::abs(v));

            expect(zeroCrossings >= 3,
                   "Tap output must oscillate within a block (got " + juce::String(zeroCrossings)
                       + " zero crossings) - too few means reads are resolving against a stale "
                         "write position rather than following the input");
            expect(peak > 0.25f, "Tap output must carry the input signal at a usable level");
        }
    }
};

static BBDDelayLineTests bbdDelayLineTests;
