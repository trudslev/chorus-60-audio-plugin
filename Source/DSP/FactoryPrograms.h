#pragma once

#include <array>

// Flat POD table of the factory programs, one field per APVTS parameter. Plain bool/float fields
// (not DSP-layer types) mirroring both siblings' own FactoryPrograms.h - keeps this header
// decoupled from the DSP classes; ProgramManager maps the fields onto the actual APVTS parameters
// when applying a program.
//
// CORE INVARIANT for this bank: every program must sound musically correct under ANY engine
// combination the user selects, not merely the one it was authored in. The engine latches are front
// -panel controls a player is expected to hit mid-performance - load 08 HALFWAY CHORUS and punch II,
// load 09 RUNAWAY and drop I - so every program carries real, considered Rate/Depth values for BOTH
// engines even when only one is engaged by default. There is no such thing here as an "unused" pair
// left at whatever the defaults happened to be: engaging a silent engine must never yield silence,
// a stale value from a previous program, or an accidental combination nobody chose.
//
// Concretely, that means 05 NEW WAVE SIX still specifies Rate/Depth I despite shipping with engine I
// off, and every engine-I-only program still specifies a deliberate Rate/Depth II.
//
// The shared/character parameters (Delay Center, Decorrelation, Drift, Saturation, Noise, Mix,
// Output Trim) are single values per program and apply unchanged across every combination - only the
// per-engine Rate/Depth pairs need to stand alone, since only they are switched in and out.
//
// This replaced an earlier 3-entry placeholder bank (I / II / I+II) that differed only in which
// engine was on and shared one set of values, which is exactly the shape this invariant rules out.
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

// Names are stored upper-case because that is how the LCD renders them ("01 EIGHTY-TWO") and how a
// host's own program menu will list them - there is no second display convention to keep in sync.
inline constexpr std::array<FactoryProgram, 9> kFactoryPrograms{ {
    // name,              eng1,  eng2,  rate1,  depth1, rate2, depth2, center, decorr, drift, sat,   noise, mix,   trim
    // Rates set by ear against the real JN-80 rather than from the original spec sheet.
    { "EIGHTY-TWO",       true,  false, 1.0f,   25.0f,  1.3f,  55.0f,  8.0f,   70.0f,  40.0f, 15.0f, 20.0f, 50.0f, 0.0f },
    { "DETUNED TWELVE",   true,  true,  0.55f,  45.0f,  1.0f,  80.0f,  8.0f,   95.0f,  40.0f, 15.0f, 20.0f, 60.0f, 0.0f },
    { "STRING MACHINE",   true,  true,  0.35f,  20.0f,  0.5f,  35.0f, 12.0f,   60.0f,  30.0f, 20.0f, 25.0f, 55.0f, 0.0f },
    { "CLEAN SWEEP",      true,  false, 0.4f,   12.0f,  1.0f,  55.0f,  7.0f,   50.0f,  25.0f, 10.0f, 15.0f, 35.0f, 0.0f },
    { "NEW WAVE SIX",     false, true,  0.55f,  25.0f,  1.6f,  60.0f,  8.0f,   75.0f,  45.0f, 40.0f, 25.0f, 55.0f, 0.0f },
    { "DOUBLING BOOTH",   true,  false, 0.5f,   15.0f,  1.0f,  55.0f,  6.0f,   10.0f,  20.0f, 10.0f, 15.0f, 30.0f, 0.0f },
    { "WHISPER WIDE",     true,  false, 0.45f,  18.0f,  1.0f,  55.0f,  8.0f,   55.0f,  10.0f,  5.0f, 10.0f, 40.0f, 0.0f },
    { "HALFWAY CHORUS",   true,  false, 0.775f, 40.0f,  1.0f,  55.0f,  8.0f,   70.0f,  40.0f, 15.0f, 20.0f, 50.0f, 0.0f },
    { "RUNAWAY",          true,  true,  0.55f,  25.0f,  1.8f,  55.0f, 10.0f,  100.0f,  95.0f, 85.0f, 70.0f, 65.0f, 0.0f },
} };

inline constexpr int kNumFactoryPrograms = (int) kFactoryPrograms.size();

// EIGHTY-TWO is the program the plugin instantiates on. Its Rate I/II are set by ear against the
// real JN-80 and so no longer equal Parameters.h's own rate defaults - which is harmless, because
// ProgramManager::initialise applies this program over the defaults on construction. The only place
// the difference shows is a knob's double-click-to-default, which resets to the parameter default
// rather than to this program's value.
inline constexpr int defaultFactoryProgramIndex = 0;
