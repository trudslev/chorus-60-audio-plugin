#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamIDs
{
    constexpr auto engine1 = "engine1";
    constexpr auto engine2 = "engine2";
    constexpr auto rate1 = "rate1";
    constexpr auto depth1 = "depth1";
    constexpr auto rate2 = "rate2";
    constexpr auto depth2 = "depth2";
    constexpr auto delayCenter = "delayCenter";
    constexpr auto decorrelation = "decorrelation";
    constexpr auto drift = "drift";
    constexpr auto saturation = "saturation";
    constexpr auto noise = "noise";
    constexpr auto mix = "mix";
    constexpr auto trim = "trim";
}

namespace LegacyMigration
{
    // Bumped whenever a stored parameter's *meaning* (not just its ID) changes incompatibly.
    // Written into getStateInformation's XML root - no remaps exist yet, adopted from day one per
    // both siblings' own Parameters.h precedent rather than retrofitted under time pressure later.
    constexpr auto stateSchemaVersionAttribute = "chorus60StateSchemaVersion";
    constexpr int currentStateSchemaVersion = 1;
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createChorus60ParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto hzAttrs = juce::AudioParameterFloatAttributes().withLabel("Hz");
    auto percentAttrs = juce::AudioParameterFloatAttributes().withLabel("%");
    auto msAttrs = juce::AudioParameterFloatAttributes().withLabel("ms");
    auto dbAttrs = juce::AudioParameterFloatAttributes().withLabel("dB");

    // Engine I/II are independent latches - both on is the real hardware's I+II trick, not a
    // separate blended mode. See BBD-TECHNICAL-NOTES.md.
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamIDs::engine1, 1}, "Chorus I", true));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamIDs::engine2, 1}, "Chorus II", false));

    // Log skew (0.3, matching both siblings' convention for frequency-ish controls) so the
    // musically-relevant low end of the range isn't crammed into a sliver of knob travel.
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::rate1, 1}, "Rate I",
        juce::NormalisableRange<float>(0.2f, 2.0f, 0.0f, 0.3f), 0.55f, hzAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::depth1, 1}, "Depth I",
        juce::NormalisableRange<float>(0.0f, 100.0f), 25.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::rate2, 1}, "Rate II",
        juce::NormalisableRange<float>(0.2f, 2.0f, 0.0f, 0.3f), 1.0f, hzAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::depth2, 1}, "Depth II",
        juce::NormalisableRange<float>(0.0f, 100.0f), 55.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::delayCenter, 1}, "Delay Center",
        juce::NormalisableRange<float>(5.0f, 15.0f), 8.0f, msAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::decorrelation, 1}, "Decorrelation",
        juce::NormalisableRange<float>(0.0f, 100.0f), 70.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::drift, 1}, "Drift",
        juce::NormalisableRange<float>(0.0f, 100.0f), 40.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::saturation, 1}, "Saturation",
        juce::NormalisableRange<float>(0.0f, 100.0f), 15.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::noise, 1}, "Noise",
        juce::NormalisableRange<float>(0.0f, 100.0f), 20.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::mix, 1}, "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f), 50.0f, percentAttrs));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::trim, 1}, "Output Trim",
        juce::NormalisableRange<float>(-24.0f, 24.0f), 0.0f, dbAttrs));

    // New parameters are appended below this line, never inserted above, to keep existing
    // sessions' and programs' parameter IDs stable.

    return {params.begin(), params.end()};
}
