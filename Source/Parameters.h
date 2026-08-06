#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// Parameter layout for CHORUS-60.
//
// The shape here follows design/BBD-TECHNICAL-NOTES-ADDENDUM.md, which corrects the original
// technical notes on a load-bearing point: I+II is NOT two engines summing. The real circuit has a
// single LFO with three structurally distinct configurations, and I+II is a third one - roughly
// twenty times faster than Chorus I, over a much narrower delay range, collapsing to mono.
//
// So there is one engine with three complete parameter sets, exactly one of which is active at a
// time, chosen by the I/II button latches:
//
//     engine1 && !engine2  ->  configuration I
//     !engine1 && engine2  ->  configuration II
//     engine1 && engine2   ->  configuration I+II
//     neither              ->  bypassed; the last page's values are retained, not reset
//
// Each configuration owns five parameters - Rate, Depth, Delay Center, Decorrelation and
// Mono/Stereo. Delay Center and Decorrelation used to be single global values, which cannot express
// the configurations' genuinely different delay ranges (I and II sit around 1.66-5.35 ms, I+II
// around 3.3-3.7 ms). Drift, Saturation, Noise, Mix and Output Trim are the genuinely global ones
// and stay shared.

namespace ParamIDs
{
    constexpr auto engine1 = "engine1";
    constexpr auto engine2 = "engine2";

    // Configuration I
    constexpr auto rate1 = "rate1";
    constexpr auto depth1 = "depth1";
    constexpr auto center1 = "center1";
    constexpr auto decorr1 = "decorr1";
    constexpr auto mono1 = "mono1";

    // Configuration II
    constexpr auto rate2 = "rate2";
    constexpr auto depth2 = "depth2";
    constexpr auto center2 = "center2";
    constexpr auto decorr2 = "decorr2";
    constexpr auto mono2 = "mono2";

    // Configuration I+II
    constexpr auto rateB = "rateB";
    constexpr auto depthB = "depthB";
    constexpr auto centerB = "centerB";
    constexpr auto decorrB = "decorrB";
    constexpr auto monoB = "monoB";

    // Global / character
    constexpr auto drift = "drift";
    constexpr auto saturation = "saturation";
    constexpr auto noise = "noise";
    constexpr auto mix = "mix";
    constexpr auto trim = "trim";
}

namespace LegacyMigration
{
    // Bumped whenever a stored parameter's *meaning* (not just its ID) changes incompatibly.
    // Written into getStateInformation's XML root.
    //
    // Version 2: the paged-engine rework. `delayCenter` and `decorrelation` were single global
    // parameters and are gone, replaced by per-configuration center1/center2/centerB and
    // decorr1/decorr2/decorrB; the Mono/Stereo switches are new; and rate1/rate2 widened from
    // 0.2-2 Hz to 0.05-8 Hz, which changes what any stored *normalised* value means. A v1 state
    // therefore cannot be read as if it were v2. No release has ever shipped v1, so this is a clean
    // break rather than a migration - but the version is recorded so that a genuine migration can
    // be written later if one is ever needed.
    constexpr auto stateSchemaVersionAttribute = "chorus60StateSchemaVersion";
    constexpr int currentStateSchemaVersion = 2;
}

namespace Chorus60Ranges
{
    // Rate I and II cover the hardware's two slow chorus configurations. The skew keeps the
    // musically dense low end off the very start of the knob's travel.
    inline juce::NormalisableRange<float> rateSlow()
    {
        return juce::NormalisableRange<float>(0.05f, 8.0f, 0.0f, 0.35f);
    }

    // Rate I+II deliberately reaches further than I and II. The factory bank specifies 9.75 Hz,
    // 11 Hz and 14 Hz here, all beyond the 8 Hz that suits the slow configurations; sharing one
    // range would clamp every one of them to 8 Hz and erase exactly the fast, near-vibrato
    // character that distinguishes I+II from a sum of I and II.
    inline juce::NormalisableRange<float> rateFast()
    {
        return juce::NormalisableRange<float>(0.05f, 16.0f, 0.0f, 0.35f);
    }

    inline juce::NormalisableRange<float> percent()
    {
        return juce::NormalisableRange<float>(0.0f, 100.0f);
    }

    // 2-14 ms spans both the I/II delay band and the narrower I+II one (3.3-3.7 ms). The previous
    // 5 ms floor could not represent the I+II centre at all.
    inline juce::NormalisableRange<float> delayCentreMs()
    {
        return juce::NormalisableRange<float>(2.0f, 14.0f);
    }
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createChorus60ParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    const auto hzAttrs = juce::AudioParameterFloatAttributes().withLabel("Hz");
    const auto percentAttrs = juce::AudioParameterFloatAttributes().withLabel("%");
    const auto msAttrs = juce::AudioParameterFloatAttributes().withLabel("ms");
    const auto dbAttrs = juce::AudioParameterFloatAttributes().withLabel("dB");

    // The switch reads MONO / STEREO on the panel rather than on/off, and a host's generic
    // parameter list should say the same thing rather than "On"/"Off".
    const auto monoAttrs = juce::AudioParameterBoolAttributes()
                               .withStringFromValueFunction([] (bool v, int) {
                                   return v ? juce::String("MONO") : juce::String("STEREO");
                               });

    // Independent latches. Both engaged is the real hardware's I+II trick and selects a distinct
    // configuration - it is not a blend of I and II.
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamIDs::engine1, 1}, "Chorus I", true));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamIDs::engine2, 1}, "Chorus II", false));

    // ---- Configuration I -------------------------------------------------------------------
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::rate1, 1}, "Rate I",
        Chorus60Ranges::rateSlow(), 0.45f, hzAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::depth1, 1}, "Depth I",
        Chorus60Ranges::percent(), 38.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::center1, 1}, "Delay Center I",
        Chorus60Ranges::delayCentreMs(), 5.6f, msAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::decorr1, 1}, "Decorrelation I",
        Chorus60Ranges::percent(), 52.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamIDs::mono1, 1}, "Mono/Stereo I", false, monoAttrs));

    // ---- Configuration II ------------------------------------------------------------------
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::rate2, 1}, "Rate II",
        Chorus60Ranges::rateSlow(), 2.90f, hzAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::depth2, 1}, "Depth II",
        Chorus60Ranges::percent(), 64.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::center2, 1}, "Delay Center II",
        Chorus60Ranges::delayCentreMs(), 4.2f, msAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::decorr2, 1}, "Decorrelation II",
        Chorus60Ranges::percent(), 66.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamIDs::mono2, 1}, "Mono/Stereo II", false, monoAttrs));

    // ---- Configuration I+II ----------------------------------------------------------------
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::rateB, 1}, "Rate I+II",
        Chorus60Ranges::rateFast(), 1.20f, hzAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::depthB, 1}, "Depth I+II",
        Chorus60Ranges::percent(), 52.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::centerB, 1}, "Delay Center I+II",
        Chorus60Ranges::delayCentreMs(), 6.4f, msAttrs));

    // Live and adjustable even though it is inaudible while this page is Mono - the page defaults
    // to Mono, but the switch is exposed, and the knob becomes meaningful the moment it is flipped.
    // Deliberately not disabled or greyed out.
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::decorrB, 1}, "Decorrelation I+II",
        Chorus60Ranges::percent(), 44.0f, percentAttrs));

    // The one configuration that defaults to Mono, matching the real circuit, which applies no
    // phase inversion in this mode.
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamIDs::monoB, 1}, "Mono/Stereo I+II", true, monoAttrs));

    // ---- Global ----------------------------------------------------------------------------
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::drift, 1}, "Drift",
        Chorus60Ranges::percent(), 22.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::saturation, 1}, "Saturation",
        Chorus60Ranges::percent(), 30.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::noise, 1}, "Noise",
        Chorus60Ranges::percent(), 14.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::mix, 1}, "Mix",
        Chorus60Ranges::percent(), 50.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::trim, 1}, "Output Trim",
        juce::NormalisableRange<float>(-12.0f, 12.0f), 0.0f, dbAttrs));

    // New parameters are appended below this line, never inserted above, to keep existing
    // sessions' and programs' parameter IDs stable.

    return {params.begin(), params.end()};
}
