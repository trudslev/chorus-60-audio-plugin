#include "BBDDelayLine.h"

void BBDDelayLine::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const int bufferSamples = (int) (0.001 * maxDelayMs * sampleRate) + 4;
    buffer.setSize((int) spec.numChannels, bufferSamples);

    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;

    for (auto& f : inputFilter)
    {
        f.prepare(monoSpec);
        *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, inputPreFilterHz);
    }

    for (auto& row : reconstructionFilter)
        for (auto& f : row)
        {
            f.prepare(monoSpec);
            *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, reconstructionFilterHz);
        }

    reset();
}

void BBDDelayLine::reset()
{
    buffer.clear();
    writeIndex.fill(0);
    for (auto& f : inputFilter)
        f.reset();
    for (auto& row : reconstructionFilter)
        for (auto& f : row)
            f.reset();
}

void BBDDelayLine::pushSample(int channel, float sample)
{
    const float filtered = inputFilter[(size_t) channel].processSample(sample);
    buffer.setSample(channel, writeIndex[(size_t) channel], filtered);
    writeIndex[(size_t) channel] = (writeIndex[(size_t) channel] + 1) % buffer.getNumSamples();
}

float BBDDelayLine::readTap(int channel, int tapIndex, float delayMs)
{
    const int numBufferSamples = buffer.getNumSamples();
    const float delaySamples = (float) (0.001 * delayMs * sampleRate);

    float readPos = (float) writeIndex[(size_t) channel] - delaySamples;
    while (readPos < 0.0f)
        readPos += (float) numBufferSamples;

    const int idx0 = (int) readPos;
    const float frac = readPos - (float) idx0;
    const int idx1 = (idx0 + 1) % numBufferSamples;

    const float raw = buffer.getSample(channel, idx0) * (1.0f - frac)
                     + buffer.getSample(channel, idx1) * frac;

    return reconstructionFilter[(size_t) channel][(size_t) tapIndex].processSample(raw);
}
