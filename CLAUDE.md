# CLAUDE.md

This file provides guidance to Claude Code when working with code in this repository.

CHORUS-60 is its own independent repo and does not depend on `../taperot/` or `../gatecrasher/` at
runtime - it's a sibling casting under the shared [Neon Foundry](../BRAND.md) umbrella, read here
purely as a structural reference (JUCE/CMake setup, APVTS conventions, DSP folder organization,
Tests/ approach, BUILDING.md shape, and - for Gatecrasher specifically - the Program storage
architecture and several GUI components ported directly rather than redesigned). Read `../BRAND.md`
first for the cross-plugin design system (naming, "Program" not "Preset", the one-accent-color rule,
component grammar), then this file for CHORUS-60's own conventions and status.
`design/CHORUS60-GUI-SPEC.md` and `design/CLAUDE.md` remain the authoritative source for exact GUI
pixel/asset detail and the sections ported from Gatecrasher; `design/BBD-TECHNICAL-NOTES.md` is the
authoritative source for the real circuit behavior the DSP layer models.

## Commands

CHORUS-60 builds on macOS (AU + VST3 + Standalone), Windows (VST3 + Standalone), and Linux
(VST3 + Standalone) — AU is Apple-only. JUCE 8.0.14 is fetched automatically via CMake
`FetchContent` on any platform, no local checkout needed.

Configure once — macOS: `cmake -B build -G Xcode`. Windows:
`cmake -B build -A x64`. Linux (single-config generator, so `CMAKE_BUILD_TYPE` must be set here
rather than only at build time): `cmake -B build -DCMAKE_BUILD_TYPE=Release`. Re-run the configure
step whenever `CMakeLists.txt` changes (new sources, new `juce_add_plugin` args) — a plain rebuild
won't pick those up.

Build: `cmake --build build --config Release`. Run the DSP unit tests (JUCE-`UnitTest`-based,
console app target `Chorus60Tests`): `./build/Tests/Chorus60Tests_artefacts/Release/Chorus60Tests`
(macOS/Linux) or the `.exe` equivalent on Windows.

See [BUILDING.md](BUILDING.md) for full per-platform requirements (Xcode/Visual Studio, CMake
3.24+, pluginval) and validation commands (`auval`/pluginval).

## Prompts log

`prompts/PROMPTS.md` holds numbered work-package prompts. Once a prompt has been fully implemented,
mark it `SHIPPED` with the date it shipped, instead of leaving it as a bare, implicitly in-flight
`PROMPT #N`.

## Architecture

### Signal chain (fixed order, all in `PluginProcessor::processBlock`)

```
Input (dry tap) ──┬────────────────────────────────────────────────────────► BBDDelayLine.pushSample
                   │                                                                    │
                   │                                        CharacterStage.advanceDrift ┤ (once/block)
                   │                                                                    │
                   │        ModulationEngine I (if Chorus I on): LFO → offset1 ─────────┤
                   │        ModulationEngine II (if Chorus II on): LFO → offset2 ───────┤
                   │                                                                    ▼
                   │                          wet[ch] = BBDDelayLine.readTap(ch,0,center+offset1+drift)
                   │                                  + BBDDelayLine.readTap(ch,1,center+offset2+drift)
                   │                                                                    │
                   │                                              StereoDecorrelationStage.process
                   │                                                                    │
                   │                                                     CharacterStage.process (sat/noise)
                   │                                                                    │
                   └──────────────────────────────────────────────────────► OutputMixStage.process → Output
```

The BBD delay line runs continuously off the (unmodulated) dry input; each engine reads its own
independently-modulated tap from the same shared buffer and the two are summed - "two genuinely
independent engines running and summing, not a third blended preset" per
`design/BBD-TECHNICAL-NOTES.md`. Engine LFO phase keeps advancing even while that engine is
disengaged, so re-engaging it never phase-jumps. Drift is a single slow-moving value recomputed
once per block (not per-sample - it retargets on a ~0.6s cycle, far slower than the audio rate) and
added into both engines' tap-position calculations before `BBDDelayLine` is read - it has to
perturb the actual delay time, not just color the audio afterward, per the technical notes'
"tiny delay jitter" description.

Each DSP stage (`Source/DSP/`) is a self-contained class with a `prepare()`/`reset()`/`process()`-
shaped interface taking plain floats/bools read from the APVTS by the processor - no DSP stage reads
the APVTS directly, except `ProgramManager`, whose entire job is APVTS manipulation (same exception
both siblings make). All stages have real DSP implemented; see BUILDING.md's "DSP tuning" note for
what's still a by-ear pass rather than a finished one.

### Parameters

`Source/Parameters.h` is the single source of truth for parameter IDs (`ParamIDs::*`) and the APVTS
layout (`createChorus60ParameterLayout()`). `PluginProcessor` caches raw atomic pointers to each
parameter in its constructor via `apvts.getRawParameterValue(...)` and reads them fresh every block
in `processBlock` - don't call `getRawParameterValue` per-block, and don't add a parameter without
adding both the layout entry and the cached pointer. New parameters are appended below the existing
list in `Parameters.h`, never inserted above, to keep saved programs' parameter IDs stable.

**Note on ranges/defaults**: `design/CHORUS60-GUI-SPEC.md` section 9's parameter table uses
different values than `Parameters.h` actually implements (e.g. its Rate range is 0.05-8Hz vs.
`Parameters.h`'s 0.2-2Hz) - this was a deliberate, explicit resolution during planning: the values
actually in `Parameters.h` are authoritative, chosen because they're the ones grounded in
`BBD-TECHNICAL-NOTES.md`'s real-hardware LFO rate ranges. The GUI's knob scaling/display is built
against `Parameters.h`, not the spec's table - don't "fix" the code to match the spec's numbers.

Every stored parameter snapshot is called a **Program**, never a "Preset" - in the UI label, the
parameter/method naming (`ProgramManager`, `FactoryProgram`, `.chorus60program`), and any docs -
matching `../BRAND.md`'s terminology rule.

### Program management (`Source/DSP/ProgramManager.*`, `FactoryPrograms.h`)

Reused directly from Gatecrasher's `ProgramManager` - same class shape, same async-safe
(`AsyncUpdater`-owning) apply path, same save-always-creates-new-never-overwrites semantics, same
per-OS user-programs directory pattern, same `LegacyMigration` schema-version seam. See
`../gatecrasher/Source/DSP/ProgramManager.h` for the original and its own more detailed rationale
comments.

**Factory bank**: 9 curated programs (see `FactoryPrograms.h`), replacing both the earlier 3-entry
I/II/I+II placeholder and the 16-name list `design/CHORUS60-GUI-SPEC.md` section 9 suggests - the
shipped bank is authoritative over that suggestion. `01 EIGHTY-TWO` is the default on instantiation.

Its **core invariant**, enforced by `Tests/FactoryProgramsTests.cpp`: every program must sound
musically correct under ANY engine combination, not just the one it was authored in. The I/II
latches are front-panel controls a player hits mid-performance, so every program carries real,
considered Rate/Depth values for BOTH engines even when only one ships engaged - engaging an idle
engine must never produce silence, a stale value from the previous program, or a combination nobody
chose. The shared/character parameters (Delay Center, Decorrelation, Drift, Saturation, Noise, Mix,
Output Trim) are single values per program and apply unchanged across every combination; only the
per-engine Rate/Depth pairs need to stand alone, since only they are switched in and out.

The LCD numbers the bank 1-based (`01 EIGHTY-TWO`), continuing past the factory entries for user
programs - the code's own indices remain 0-based.

### GUI (`Source/GUI/`)

Asset-based for the *sculpted* elements - the chassis and the knobs - matching Gatecrasher's own
approach and for the same stated reason: pre-rendered bitmap sculpting reads as more authentically
"real 80s/90s hardware" than modern vector rendering. Everything flat is drawn in code.

The background is `design/assets/chorus60-background-plate@2x.png`: the bare fascia - panel material
and frame, header chrome with empty PROGRAM / IN / OUT wells, the blue stripes and blank CHORUS
strip, section divider, empty scope well, and the empty group boxes with their heading rules. Per
the spec's preamble, "every glyph on the panel is drawn by the host, not baked in". On top of it:
`PanelChrome` (the static silkscreened layer - model lines, stripe caption, group titles, knob name
labels, IN/OUT captions, footer status line), then the live pieces: the 11 knobs (bitmap filmstrips -
literally the same two filmstrip files Gatecrasher ships, shared across both plugins' design assets)
with their live value labels, the engine buttons and all six LEDs, the delay-modulation
oscilloscope, the program LCD's dynamic text, and the IN/OUT meter readouts.

This replaced `chorus60-panel-bypass@2x.png`, a fully dressed render, as the background. Compositing
live elements over that meant each one sat on a baked copy of itself - knob filmstrips over baked
knobs whose frozen pointers showed through, live readouts over stale numbers. Gatecrasher hit the
same wall across its whole GUI and needed a bare chassis to get out of it; the dressed renders stay
in `design/assets/` as pixel-matching acceptance targets. Type sizes are quoted by the spec as CSS
px, which is **not** a `juce::Font` height (that's ascent+descent) - use
`Chorus60Theme::labelFontHeightForCssPx` rather than passing spec numbers straight to `labelFont()`.

Several components are ported directly from Gatecrasher rather than rewritten, per `design/CLAUDE.md`'s explicit instruction:
`ProgramHeader` (identical contract, Gatecrasher's own words), `KnobFilmstripComponent`, and the
theme/LookAndFeel's caching patterns. `EngineButtonComponent` (the I/II/OFF hardware buttons) is
new - there's no Gatecrasher equivalent - styled directly against
`design/assets/jn80-chorus-reference.jpeg`, the real hardware photo.

`design/CHORUS60-GUI-SPEC.md` is the authoritative pixel spec - read it before touching any GUI
code, including its section 12, which documents and explicitly accepts two deliberate departures
from `BRAND.md`'s literal component grammar (the button colors, the structural blue stripes) - don't
"correct" those. `design/CLAUDE.md` is Claude Design's handoff note summarizing the same. Both were
written before implementation began and are meant to be implemented as-is, not redesigned.

### Build system

`CMakeLists.txt` fetches JUCE via `FetchContent` (pinned to `8.0.14`, matching both siblings so the
three plugins don't drift on JUCE version) and defines one `juce_add_plugin(Chorus60 ...)` target,
with `FORMATS`/copy-dir branching on `if(APPLE)` the same way both siblings' do.
`PLUGIN_MANUFACTURER_CODE` (`Nfdy`), `PLUGIN_CODE` (`Ch60`, referencing the design spec's own
"MODEL CH-60" tagline), `BUNDLE_ID` (`com.neonfoundry.chorus60`) and `COMPANY_NAME`
("Neon Foundry") are settled, not placeholders - the vendor identity was unified across the suite
before any versioned release, and changing it again breaks saved projects in both AU and VST3 (JUCE
derives the VST3 class ID from the manufacturer and plugin codes together). `Tests/` is a separate
`juce_add_console_app` target that compiles the DSP `.cpp` files directly (not linked against the
plugin target) plus its own JUCE-`UnitTest` files - new DSP `.cpp` files need to be added to both
`target_sources(Chorus60 ...)` here and `target_sources(Chorus60Tests ...)` in `Tests/CMakeLists.txt`
to be covered by tests.

Unlike Gatecrasher, CHORUS-60 has no sidechain input bus - `BusesProperties` is plain stereo in/out.

## Status

- **DSP**: every stage has real, functioning processing - no stubs, all grounded directly in
  `design/BBD-TECHNICAL-NOTES.md`'s description of the real circuit. Exact filter cutoffs and
  character-parameter ranges are a first, technically-reasoned pass rather than a tuned one - see
  `BUILDING.md`'s DSP tuning note. Build, load, listen, adjust.
- **GUI**: implemented against the approved spec using the bare-background-plate approach above,
  with `ProgramHeader`/`KnobFilmstripComponent`/theme caching ported directly from Gatecrasher per
  `design/CLAUDE.md`'s explicit instruction.
- **Program bank**: 9 curated programs, `01 EIGHTY-TWO` default. Values are structurally verified
  (ranges, and the both-engines invariant) but the bank has not had a by-ear pass - in particular
  whether engaging the *idle* engine on each program sounds intentional.
- **Program paradigm matches Gatecrasher exactly**: click the LCD for the program menu; SAVE is
  disabled until a parameter actually differs from the loaded program, DELETE is disabled for
  factory programs, and Save always creates a new user program rather than overwriting.
