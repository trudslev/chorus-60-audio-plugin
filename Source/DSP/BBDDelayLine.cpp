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

    // The requested delay is clamped to a window the buffer can actually satisfy. A tap must never
    // read at or past the write head: at delayMs <= 0 the read position lands on (or ahead of) the
    // sample just written, which after the modulo wrap means stale audio from a whole buffer ago -
    // an instant discontinuity, not a short delay. That is reachable in normal use, because the tap
    // position is Delay Center plus a signed modulation excursion plus drift: at minimum Delay
    // Center (5ms) with Depth at 100% (+-5ms excursion) the trough of the sweep reaches zero.
    // Clamping here rather than at the call site keeps the invariant with the buffer that owns it,
    // so no caller can violate it. The audible result at those settings is that the deepest part of
    // the sweep flattens against the floor instead of glitching.
    const float clampedDelayMs = juce::jlimit(minDelayMs, maxDelayMs - 1.0f, delayMs);
    const float delaySamples = (float) (0.001 * clampedDelayMs * sampleRate);

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
