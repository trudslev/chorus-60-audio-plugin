#pragma once

#include <array>

// Flat POD table of the factory programs, one field per APVTS parameter. Plain bool/float fields
// (not DSP-layer types) mirroring both siblings' own FactoryPrograms.h - keeps this header
// decoupled from the DSP classes; ProgramManager maps the fields onto the actual APVTS parameters
// when applying a program.
//
// This is the baseline 3-program pass only (I / II / I+II), all sharing the same shared-parameter
// defaults from Parameters.h and differing only in which engine(s) are on - the full curated
// 16-name factory bank (see design/CHORUS60-GUI-SPEC.md section 9 for the suggested list: Wide
// Ensemble, Juno I, Juno II, Juno I+II, Slow Swell, Vibrato, Dimension, Shimmer Pad, Clock Noise,
// Warped Tape, Deep Detune, String Machine, Bright Doubler, Mono Verify, Cold Chorus, Dark
// Ensemble) is an explicit follow-up, not implemented here - see prompts/PROMPTS.md.
struct FactoryProgram
{
    const char* name;

    bool engine1;
    bool engine2;
    float rate1Hz;
    float depth1Percent;
    float rate2Hz;
    float depth2Percent;
    float delayCenterMs;
    float decorrelationPercent;
    float driftPercent;
    float saturationPercent;
    float noisePercent;
    float mixPercent;
    float trimDb;
};

inline constexpr std::array<FactoryProgram, 3> kFactoryPrograms{ {
    // name,      eng1,  eng2,  rate1, depth1, rate2, depth2, center, decorr, drift, sat,  noise, mix,  trim
    { "I",        true,  false, 0.55f, 25.0f,  1.0f,  55.0f,  8.0f,   70.0f,  40.0f, 15.0f, 20.0f, 50.0f, 0.0f },
    { "II",       false, true,  0.55f, 25.0f,  1.0f,  55.0f,  8.0f,   70.0f,  40.0f, 15.0f, 20.0f, 50.0f, 0.0f },
    { "I + II",   true,  true,  0.55f, 25.0f,  1.0f,  55.0f,  8.0f,   70.0f,  40.0f, 15.0f, 20.0f, 50.0f, 0.0f },
} };

inline constexpr int kNumFactoryPrograms = (int) kFactoryPrograms.size();

// "I" is already identical to Parameters.h's own defaults (engine1=on, engine2=off), so it's the
// natural default landing program - no separate rationale needed the way TapeRot/Gatecrasher's
// picks required.
inline constexpr int defaultFactoryProgramIndex = 0;
