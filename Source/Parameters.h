#pragma once

#include "DSP/FactoryPrograms.h"

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
    constexpr auto image1 = "image1";

    // Configuration II
    constexpr auto rate2 = "rate2";
    constexpr auto depth2 = "depth2";
    constexpr auto center2 = "center2";
    constexpr auto decorr2 = "decorr2";
    constexpr auto image2 = "image2";

    // Configuration I+II
    constexpr auto rateB = "rateB";
    constexpr auto depthB = "depthB";
    constexpr auto centerB = "centerB";
    constexpr auto decorrB = "decorrB";
    constexpr auto imageB = "imageB";

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
    // therefore cannot be read as if it were v2.
    //
    // Version 3: the revision-2 panel. `mono1`/`mono2`/`monoB` are renamed `image1`/`image2`/
    // `imageB` (spec section 7.2 - the control is IMAGE, MONO and STEREO are its positions), and
    // rate1/rate2 widen from 0.05-8 Hz to 0.05-16 Hz to match rateB and the one printed scale the
    // plate now carries for all three pages. A v2 state read as v3 would silently lose all three
    // switches to their defaults and place both slow Rates at the wrong rotation.
    //
    // No release has ever shipped any of these, so each bump is a clean break rather than a
    // migration - but the version is recorded so a genuine migration can be written later if one is
    // ever needed.
    constexpr auto stateSchemaVersionAttribute = "chorus60StateSchemaVersion";
    constexpr int currentStateSchemaVersion = 4;

    /** The schema at which the session stopped storing a positional index and started storing bank
        + identifier. */
    constexpr int identitySchemaVersion = 4;

    /** **The oldest schema whose values can still be interpreted, pinned to a literal.**

        v1 predates the paged-engine rework: delayCenter and decorrelation were single globals and
        rate1/rate2 spanned 0.2-2 Hz rather than 0.05-8 Hz, so a stored NORMALISED value means
        something different there. There is nothing to salvage, and loading it would produce
        plausible wrong values rather than an obvious fallback.

        v2 and v3 are readable: v3 added identity attributes, which is purely additive.

        **This is a literal on purpose.** The gate used to read `savedSchema != current`, which was
        correct exactly once - every schema bump then discarded the previous version's sessions
        wholesale, including ones whose parameters had not changed meaning at all. Gatecrasher had
        the same shape in its algorithm remap and it silently rotated the choice on every load. */
    constexpr int oldestReadableSchemaVersion = 2;

    /** **The identity attributes, and they are a contract.** Rename one and the session still
        parses while the Program silently reverts, with no error anywhere. `...ProgramName` is
        DISPLAY ONLY - it names an unresolved identifier on the panel and resolves nothing. */
    constexpr auto programBankAttribute = "chorus60ProgramBank";
    constexpr auto programIdAttribute   = "chorus60ProgramId";
    constexpr auto programNameAttribute = "chorus60ProgramName";

    inline juce::String bankAttributeValue (ProgramBank bank)
    {
        switch (bank)
        {
            case ProgramBank::init:       return "init";
            case ProgramBank::factory:    return "factory";
            case ProgramBank::user:       return "user";
            case ProgramBank::unresolved: return "unresolved";
        }

        return "factory";
    }

    inline ProgramBank bankFromAttribute (const juce::String& value)
    {
        if (value == "init")       return ProgramBank::init;
        if (value == "user")       return ProgramBank::user;
        if (value == "unresolved") return ProgramBank::unresolved;

        return ProgramBank::factory;
    }

    /** How a stored schema relates to what this build understands. Three outcomes, deliberately
        distinct: too old to interpret, too new to know about, or usable. */
    enum class SchemaVerdict { tooOld, tooNew, readable };

    inline SchemaVerdict classifySchema (int savedSchema) noexcept
    {
        if (savedSchema < oldestReadableSchemaVersion) return SchemaVerdict::tooOld;
        if (savedSchema > currentStateSchemaVersion)   return SchemaVerdict::tooNew;

        return SchemaVerdict::readable;
    }
}

namespace Chorus60Ranges
{
    // ONE Rate range for all three configurations. What distinguishes I, II and I+II is the values
    // the factory Programs store, not what their controls can reach - a player who wants I running
    // at 12 Hz should be able to have it.
    //
    // Resolved upward rather than by narrowing I+II: the bank specifies 9.75, 11 and 14 Hz there,
    // and a shared 8 Hz ceiling would clamp every one of them and erase the fast, near-vibrato
    // character that distinguishes I+II from a sum of I and II.
    //
    // The skew is what makes the pointer agree with the printed scale, and it is exact rather than
    // approximate. JUCE's convertTo0to1 is ((v-start)/(end-start))^skew, so 0.35 over 0.05-16 puts
    // the spec's five marks (section 7.1) at:
    //
    //     0.05 Hz -> -135.00 deg      spec -135.0
    //     0.5  Hz ->  -57.55          spec  -57.5
    //     2    Hz ->   -5.61          spec   -5.7
    //     8    Hz ->  +76.61          spec  +76.7
    //     16   Hz -> +135.00          spec +135.0
    //
    // Worst disagreement 0.09 degrees - a twentieth of a pixel at the r=47 tick radius, and well
    // inside the 128-frame filmstrip's own +/-1.06 degree quantisation. Section 7.1 calls the taper
    // "piecewise log interpolation through those five anchors"; it is this same curve, so there is
    // no lookup table to build. Verified by measuring the baked ticks off the plate.
    //
    // Because all three now share it, no numeral on the panel is page-dependent, which is the
    // reason the scales could be baked into the plate at all.
    inline juce::NormalisableRange<float> rate()
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
    // The control is IMAGE; MONO and STEREO are its two positions, printed beside the thumb and
    // reported to the host as the values (spec section 7.2). true = MONO.
    const auto imageAttrs = juce::AudioParameterBoolAttributes()
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
        Chorus60Ranges::rate(), 0.45f, hzAttrs));

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
        juce::ParameterID{ParamIDs::image1, 1}, "Image I", false, imageAttrs));

    // ---- Configuration II ------------------------------------------------------------------
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::rate2, 1}, "Rate II",
        Chorus60Ranges::rate(), 2.90f, hzAttrs));

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
        juce::ParameterID{ParamIDs::image2, 1}, "Image II", false, imageAttrs));

    // ---- Configuration I+II ----------------------------------------------------------------
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::rateB, 1}, "Rate I+II",
        Chorus60Ranges::rate(), 1.20f, hzAttrs));

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
        juce::ParameterID{ParamIDs::imageB, 1}, "Image I+II", true, imageAttrs));

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
