Convention: once a prompt below has been implemented, mark it SHIPPED with the date, e.g.
"PROMPT #1 - SHIPPED 2026-08-04".

PROMPT #1 - SHIPPED 2026-08-04

Scaffold Chorus-60, a Juno-60/JN-80-style BBD ensemble chorus, as a new sibling plugin to TapeRot
and Gatecrasher under neon-foundry/. Study TapeRot's Source/ layout, DSP folder organization (one
class per responsibility), Tests/ setup, and BUILDING.md format, and reuse Gatecrasher's now-proven
Program storage architecture (Factory/User banks, save-always-creates-new, FACT/USER LCD indicator)
directly rather than redesigning it. Asset-based GUI, same approach and reasoning as Gatecrasher,
per design/CHORUS60-GUI-SPEC.md and design/CLAUDE.md (already authored by Claude Design) - several
components (ProgramHeader, KnobFilmstripComponent, theme caching patterns) explicitly ported from
Gatecrasher rather than rewritten. DSP architecture grounded directly in design/BBD-TECHNICAL-NOTES.md's
description of the real circuit's specific imperfections (rounded/asymmetric LFO, small delay
excursion, genuine L/R decorrelation rather than panning, fixed ~6-8kHz reconstruction-filter
rolloff, drift/saturation/noise character).

Delivered: full project scaffold (CMakeLists.txt, .gitignore, README.md, BUILDING.md, CLAUDE.md);
Parameters.h with all 13 parameters (using the explicitly-corrected ranges/defaults grounded in the
technical notes' real-hardware LFO rates, not design/CHORUS60-GUI-SPEC.md's own placeholder table -
see CLAUDE.md's Parameters section for why); the DSP signal chain (ModulationEngine x2,
BBDDelayLine, StereoDecorrelationStage, CharacterStage, OutputMixStage); ProgramManager +
FactoryPrograms.h with the 3 baseline programs (I, II, I+II - save-always-creates-new, never
overwrites, no "New Program" button); PluginProcessor/PluginEditor wiring it all together; the GUI
layer (Chorus60PanelBackground, KnobFilmstripComponent, ModScope, EngineButtonComponent,
ProgramHeader, WordmarkComponent, Chorus60EditorContent); and a JUCE-UnitTest DSP test suite.

PROMPT #2

Follow-up work identified during PROMPT #1, not yet done:

1. DSP tuning by ear. ModulationEngine's corner-smoothing time constant and rise/fall asymmetry,
   BBDDelayLine's input-prefilter/reconstruction-filter cutoffs, CharacterStage's drift/saturation/
   noise-floor ranges, and StereoDecorrelationStage's max extra-delay/invert-blend amounts are all a
   first, technically-reasoned pass grounded in BBD-TECHNICAL-NOTES.md's prose description, not a
   tuned one - same status both siblings' own DSP had before their by-ear pass. Build, load each of
   the 3 programs, listen, adjust. Pay particular attention to whether Engine I actually reads as
   subtle "one oscillator sounds like three" and Engine II as more audible ensemble motion (per the
   technical notes' own framing), and whether I+II produces the described quasi-periodic, never-
   quite-repeating movement rather than just sounding like more of the same.

2. The full curated factory bank. Only 3 baseline programs (I, II, I+II) are implemented -
   design/CHORUS60-GUI-SPEC.md section 9 suggests 16 names for the real bank: Wide Ensemble, Juno I,
   Juno II, Juno I+II, Slow Swell, Vibrato, Dimension, Shimmer Pad, Clock Noise, Warped Tape, Deep
   Detune, String Machine, Bright Doubler, Mono Verify, Cold Chorus, Dark Ensemble. Design each
   program's parameter values by ear once the DSP tuning pass (item 1) is done - don't tune the bank
   against untuned DSP.

3. DONE. PLUGIN_MANUFACTURER_CODE is Nfdy (shared across the suite), PLUGIN_CODE Ch60, BUNDLE_ID
   com.neonfoundry.chorus60, COMPANY_NAME "Neon Foundry". Previously
   placeholders, effectively permanent once shipped or automated against.

4. No LICENSE.txt/OFL notice ships alongside the Barlow Condensed / Share Tech Mono / Librestile
   Extended Bold fonts in design/assets/ yet (same gap as Gatecrasher's) - all are open-license,
   free for commercial use, but the notice files themselves are missing. TapeRot's
   design/inter/LICENSE.txt is the precedent to match.
