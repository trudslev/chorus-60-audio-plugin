#include "ModulationEngine.h"
#include <cmath>
#include <juce_core/juce_core.h>

namespace
{
    // Deliberately not 50/50: real analog integrator/comparator LFO circuits don't have a
    // perfectly symmetric rise/fall, and a flat asymmetry stops the modulation from feeling
    // mechanically repetitive (see BBD-TECHNICAL-NOTES.md's "tiny imperfections" note).
    constexpr float riseFraction = 0.55f;

    // Corner-rounding time constant - short relative to the ~0.5-2s LFO period, just enough to
    // remove the triangle's sharp direction changes ("very smooth modulation... no abrupt
    // direction changes" per the notes) without eating the fundamental shape.
    constexpr double cornerSmoothingSeconds = 0.05;

    // "Only a couple of milliseconds" of excursion even at full depth - much tighter than a modern
    // chorus's 5-25ms swing, per the notes' Delay Modulation section.
    // Doubled from 2.5ms after listening against the real JN-80 - the original ceiling didn't travel
    // far enough to reach the deeper end of what the hardware does. BBDDelayLine::readTap clamps the
    // resulting tap position, so a deep sweep against a low Delay Center flattens at the buffer's
    // floor rather than wrapping - see that clamp's comment.
    constexpr float maxExcursionMs = 5.0f;
}

void ModulationEngine::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    smoothingCoeff = 1.0f - std::exp((float) (-1.0 / (cornerSmoothingSeconds * sampleRate)));
    reset();
}

void ModulationEngine::reset()
{
    phase = 0.0f;
    smoothedLfo = 0.0f;
}

float ModulationEngine::nextLfoValue(float rateHz)
{
    float raw;
    if (phase < riseFraction)
        raw = -1.0f + 2.0f * (phase / riseFraction);
    else
        raw = 1.0f - 2.0f * ((phase - riseFraction) / (1.0f - riseFraction));

    phase += (float) (rateHz / sampleRate);
    if (phase >= 1.0f)
        phase -= 1.0f;

    smoothedLfo += smoothingCoeff * (raw - smoothedLfo);
    return smoothedLfo;
}

float ModulationEngine::getNextOffsetMs(float rateHz, float depthPercent)
{
    const float depth01 = juce::jlimit(0.0f, 1.0f, depthPercent * 0.01f);
    return nextLfoValue(rateHz) * depth01 * maxExcursionMs;
}
