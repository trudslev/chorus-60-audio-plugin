#pragma once

#include <array>

// Flat POD table of the factory programs, one field per APVTS parameter. Plain bool/float fields
// (not DSP-layer types) mirroring both siblings' own FactoryPrograms.h - keeps this header
// decoupled from the DSP classes; ProgramManager maps the fields onto the actual APVTS parameters
// when applying a program.
//
// CORE INVARIANT for this bank: every program must sound musically correct under ANY engine
// combination the user selects, not merely the one it was authored in. The engine latches are
// front-panel controls a player is expected to hit mid-performance - load 08 HALFWAY CHORUS and
// punch II, load 09 RUNAWAY and drop I - so every program carries a real, considered value for all
// three configurations even when only one is engaged by default. There is no such thing here as an
// "unused" configuration left at whatever the defaults happened to be: selecting a configuration
// must never yield silence, a stale value from a previous program, or an accidental combination
// nobody chose.
//
// Concretely: 05 NEW WAVE SIX still specifies a full configuration I despite shipping with engine I
// off, and every engine-I-only program still specifies a deliberate configuration II.
//
// Each of the three configurations owns its own Rate, Depth, Delay Center, Decorrelation and
// Mono/Stereo. Delay Center and Decorrelation were single shared values before the correction in
// design/BBD-TECHNICAL-NOTES-ADDENDUM.md; they are per-configuration now because the three
// configurations sit in genuinely different delay bands (I and II around 1.66-5.35 ms, I+II in the
// much narrower 3.3-3.7 ms).
//
// Note the I+II rates: 9.75 Hz, 11 Hz and 14 Hz. These sit far above the 8 Hz ceiling that suits I
// and II, which is precisely why rateB carries its own wider 0.05-16 Hz range - see Parameters.h.
// Sharing one range would clamp all of them to 8 Hz and flatten the distinction this bank relies
// on.
struct FactoryConfiguration
{
    float rateHz;
    float depthPercent;
    float delayCentreMs;
    float decorrelationPercent;
    // The IMAGE switch's stored position. The control is IMAGE (spec section 7.2); MONO and STEREO
    // are its values, and true means MONO - so this field is named for both halves of that.
    bool  imageMono;
};

struct FactoryProgram
{
    const char* name;

    // Which engines are latched on when the program loads. All three configurations below are
    // populated regardless - see the invariant above.
    bool engine1;
    bool engine2;

    FactoryConfiguration configI;
    FactoryConfiguration configII;
    FactoryConfiguration configBoth;

    float driftPercent;
    float saturationPercent;
    float noisePercent;
    float mixPercent;
    float trimDb;
};

// Names are stored upper-case because that is how the LCD renders them ("01 EIGHTY-TWO") and how a
// host's own program menu will list them - there is no second display convention to keep in sync.
//
// Several I and II rates are the real circuit's measured figures (0.513 Hz for Chorus 1, 0.863 Hz
// for Chorus 2) rather than round numbers. That is deliberate, not a typo.
inline constexpr std::array<FactoryProgram, 9> kFactoryPrograms{ {
    //                     eng1   eng2   configuration I                            configuration II                           configuration I+II                          drift  sat    noise  mix    trim
    { "EIGHTY-TWO",        true,  false, { 0.55f,  25.0f,  8.0f,  70.0f, false }, { 1.0f,   55.0f,  8.0f,  70.0f, false }, {  9.75f,  50.0f, 3.5f,  70.0f, true }, 40.0f, 15.0f, 20.0f, 50.0f, 0.0f },
    { "DETUNED TWELVE",    true,  true,  { 0.513f, 70.0f,  8.0f, 100.0f, false }, { 0.863f, 85.0f,  8.0f, 100.0f, false }, { 11.0f,   80.0f, 3.5f, 100.0f, true }, 40.0f, 15.0f, 20.0f, 60.0f, 0.0f },
    { "STRING MACHINE",    true,  true,  { 0.35f,  20.0f, 12.0f,  60.0f, false }, { 0.5f,   35.0f, 12.0f,  60.0f, false }, {  9.75f,  50.0f, 3.5f,  60.0f, true }, 30.0f, 20.0f, 25.0f, 55.0f, 0.0f },
    { "CLEAN SWEEP",       true,  false, { 0.4f,   12.0f,  7.0f,  50.0f, false }, { 1.0f,   55.0f,  7.0f,  50.0f, false }, {  9.75f,  50.0f, 3.5f,  50.0f, true }, 25.0f, 10.0f, 15.0f, 35.0f, 0.0f },
    { "NEW WAVE SIX",      false, true,  { 0.55f,  25.0f,  8.0f,  75.0f, false }, { 1.6f,   60.0f,  8.0f,  75.0f, false }, {  9.75f,  50.0f, 3.5f,  75.0f, true }, 45.0f, 40.0f, 25.0f, 55.0f, 0.0f },
    { "DOUBLING BOOTH",    true,  false, { 0.5f,   15.0f,  6.0f,  10.0f, false }, { 1.0f,   55.0f,  6.0f,  10.0f, false }, {  9.75f,  50.0f, 3.5f,  10.0f, true }, 20.0f, 10.0f, 15.0f, 30.0f, 0.0f },
    { "WHISPER WIDE",      true,  false, { 0.45f,  18.0f,  8.0f,  55.0f, false }, { 1.0f,   55.0f,  8.0f,  55.0f, false }, {  9.75f,  50.0f, 3.5f,  55.0f, true }, 10.0f,  5.0f, 10.0f, 40.0f, 0.0f },
    { "HALFWAY CHORUS",    true,  false, { 0.775f, 40.0f,  8.0f,  70.0f, false }, { 1.0f,   55.0f,  8.0f,  70.0f, false }, {  9.75f,  50.0f, 3.5f,  70.0f, true }, 40.0f, 15.0f, 20.0f, 50.0f, 0.0f },
    { "RUNAWAY",           true,  true,  { 0.513f, 90.0f, 10.0f, 100.0f, false }, { 0.863f, 90.0f, 10.0f, 100.0f, false }, { 14.0f,  100.0f, 3.5f, 100.0f, true }, 95.0f, 85.0f, 70.0f, 65.0f, 0.0f },
} };

inline constexpr int kNumFactoryPrograms = (int) kFactoryPrograms.size();

// EIGHTY-TWO is the program the plugin instantiates on. Its values deliberately differ from
// Parameters.h's own defaults, which is harmless because ProgramManager::initialise applies this
// program over the defaults on construction. The only place the difference shows is a knob's
// double-click-to-default, which resets to the parameter default rather than to this program's
// value.
inline constexpr int defaultFactoryProgramIndex = 0;
