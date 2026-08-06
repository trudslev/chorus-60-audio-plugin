#include "ModulationEngine.h"
#include <cmath>
#include <juce_core/juce_core.h>

namespace
{
    // Deliberately not 50/50: real analog integrator/comparator LFO circuits don't have a
    // perfectly symmetric rise/fall, and a flat asymmetry stops the modulation from feeling
    // mechanically repetitive (see BBD-TECHNICAL-NOTES.md's "tiny imperfections" note).
    constexpr float riseFraction = 0.55f;

    // Corner-rounding time constant - short relative to the LFO period, just enough to remove the
    // triangle's sharp direction changes ("very smooth modulation... no abrupt direction changes"
    // per the notes) without eating the fundamental shape.
    //
    // 50ms was chosen against the sub-2Hz rates of configurations I and II, where it is a small
    // fraction of a 0.5-2s period. It cannot stay fixed now that I+II runs at 9.75-14Hz: as a
    // one-pole its corner sits near 3.2Hz, so at 9.75Hz it would attenuate the modulation to about
    // a third and at 14Hz to about a quarter. The fast configuration would come out *weaker* than
    // the slow ones rather than faster - precisely inverting what the addendum describes.
    //
    // So the constant holds at 50ms up to 2Hz, which leaves every rate configurations I and II
    // actually use (0.35-1.8Hz in the factory bank) bit-identical to before, and above that scales
    // as 1/rate to keep the same proportion of the cycle. At 9.75Hz that is ~10ms (corner ~15Hz)
    // and at 14Hz ~7ms (corner ~22Hz) - still rounding the corners, no longer swallowing the shape.
    constexpr double cornerSmoothingSecondsMax = 0.05;
    constexpr double cornerSmoothingRateProduct = 0.1; // = cornerSmoothingSecondsMax * 2Hz

    double cornerSmoothingSecondsFor(float rateHz)
    {
        if (rateHz <= 2.0f)
            return cornerSmoothingSecondsMax;
        return cornerSmoothingRateProduct / (double) rateHz;
    }

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

    // Recomputed per sample rather than cached in prepare(): the coefficient now depends on Rate,
    // which is a live parameter and, unlike sample rate, changes while audio is running - including
    // discontinuously when the engaged configuration changes from a slow page to the fast I+II one.
    const float coeff =
        1.0f - std::exp((float) (-1.0 / (cornerSmoothingSecondsFor(rateHz) * sampleRate)));

    smoothedLfo += coeff * (raw - smoothedLfo);
    return smoothedLfo;
}

float ModulationEngine::getNextOffsetMs(float rateHz, float depthPercent)
{
    const float depth01 = juce::jlimit(0.0f, 1.0f, depthPercent * 0.01f);
    return nextLfoValue(rateHz) * depth01 * maxExcursionMs;
}
