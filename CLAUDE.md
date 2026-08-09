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

**The diagram above is one engine, not two.** `PluginProcessor` owns a single `ModulationEngine`
("Singular, deliberately." - `PluginProcessor.h`), and I / II / I+II are three *configurations* of it,
each with its own full parameter set, exactly one of which is resolved per block by
`resolveActiveConfiguration()`. An earlier draft of this file described two engines summing two taps,
which the `BBD-TECHNICAL-NOTES-ADDENDUM.md` rework superseded. Delay Center and Decorrelation are
per-configuration too (`center1/2/B`, `decorr1/2/B`), not single global values. Engine LFO phase keeps advancing even while that engine is
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

**Rate: one range, one skew, and the skew is load-bearing.** All three configurations use
`Chorus60Ranges::rate()` - **0.05-16 Hz, skew 0.35**. They used to differ, with I and II capped at
8 Hz, which could not survive revision 2: the plate prints ONE 0.05-16 legend that whichever page is
showing reads against, so a narrower range on I or II put its pointer at the wrong printed numeral
for every value. What distinguishes the three configurations is the values the factory bank stores,
not what their controls can reach.

The skew is not a taste setting. JUCE's `convertTo0to1` is `((v-start)/(end-start))^skew`, and 0.35
over 0.05-16 puts spec section 7.1's five printed marks at -135.00 / -57.55 / -5.61 / +76.61 /
+135.00 degrees against the spec's -135.0 / -57.5 / -5.7 / +76.7 / +135.0 - worst disagreement 0.09
degrees, a twentieth of a pixel at the tick radius, and verified by measuring the baked ticks off the
plate. Section 7.1 describes the taper as "piecewise log interpolation through those five anchors";
it is this same curve, so **there is no lookup table to build**. `ParametersStateTests` asserts the
range, the skew and all three marks, so changing any of them fails a test rather than silently
mis-aligning the panel.

**The IMAGE switch**: `image1`/`image2`/`imageB`, display names `IMAGE I`/`II`/`I+II`, values still
`MONO`/`STEREO` (true = MONO). The control is IMAGE; MONO and STEREO are its printed positions, the
same way Gatecrasher's KEY SOURCE prints INTERNAL/SIDECHAIN. Renamed from `mono*` in schema 3.

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
chose. The genuinely global parameters (Drift, Saturation, Noise, Mix, Output Trim) apply unchanged
across every combination; every configuration
carries its own complete set, so switching pages never inherits a value from the page before it.

The LCD numbers the bank 1-based (`01 EIGHTY-TWO`), continuing past the factory entries for user
programs - the code's own indices remain 0-based.

### GUI (`Source/GUI/`)

Asset-based for the *sculpted* elements - the plate, the knobs, the buttons, the lamps and the switch
- matching Gatecrasher's approach and for the same reason: pre-rendered bitmap sculpting reads as more
authentically "real 80s/90s hardware" than modern vector rendering. Everything flat is drawn in code.

**Revision 2 inverted what "flat" covers.** The plate used to be bare material with every glyph drawn
in code. It now BAKES the printed scales, every tick ring, numeral and unit, the wordmark, the model
line, the group headings, the five global knob labels, the switch's printed STEREO/MONO positions and
the PROGRAM/IN/OUT captions - plus empty wells for the scope, the LCD and the two meters.
`design/CHORUS60-BUILD-HANDOFF.md` section 1 is the asset contract and it cuts both ways: **redrawing
a baked string double-prints it at a one-pixel offset; baking a runtime one freezes it.**

Only these are drawn at runtime:

| Element | Where |
|---|---|
| Knob sprites | `KnobFilmstripComponent` |
| Engine button faces, lamps, IMAGE switch | `EngineButtonComponent`, `ImageSwitch` |
| Scope trace, grid, annotations, status row | `ModScope` |
| LCD: tag, program name, parameter readout, caret, chevron | `ProgramHeader` |
| IN / OUT numerals, SAVE / DELETE | `ProgramHeader` |
| MOD ENGINE heading + status note, footer | `Chorus60EditorContent::paintOverChildren` |
| The five page-suffixed slot labels | `ModSlotLabels` |

`PanelChrome` and `WordmarkComponent` are **deleted**, not adapted - everything they drew is
silkscreen now. So is `KnobValueLabel`, the standing value under every knob, and the drag-time popup:
revision 2 removed all standing readouts, and the LCD is the only numeric display on the panel. The
knob tick ring went the same way: revision 1 drew it at even angular spacing, and the plate now prints
every tick at its *labelled* value, which on the skewed Rate knob is not evenly spaced. **There is no
mark table in the code** - the plate is the single source of truth for where a mark sits.

**Coordinates are inside-border.** The exported plate is 1282 x 777 including a 1 px frame, and both
design documents measure from the first pixel of panel material inside it. `PluginEditor` draws the
plate at full canvas size and places `Chorus60EditorContent` at (1, 1) sized 1280 x 775, so every
`Layout` constant is a literal spec value with no arithmetic hanging off it.

**The OFF state is a multiply, and the composition order is what makes it one.** Section 9 dims the
three group boxes - MOD ENGINE, CHARACTER, OUTPUT, each from its heading rule down - by 0.50; the
header, scope, button column and footer stay at full brightness. `DimmableGroup` implements it: an
opaque black rect painted *outside* the fade, and above it a child holding that group's own crop of
the plate plus its controls, carrying `setAlpha(k)`. JUCE flattens that subtree once, so over black
the result is exactly `k*src`. Read that class's comment before touching it - `setAlpha` on a child
sitting over the already-painted plate blends toward the plate instead, which is the treatment
section 9 explicitly rejects, and per-element opacity breaks wherever two elements overlap. Verified
by measurement: median ratio 0.500 inside the three boxes and 1.000 everywhere else, matching
`chorus60-page-off@2x.png`.

Two traps that cost time here:

- `setInterceptsMouseClicks(false, false)` on a container **blocks its children too**. The second
  argument is `allowClicksOnChildComponents`; with it false every knob inside a DimmableGroup went
  dead while still repainting perfectly.
- Type sizes are quoted by the spec as CSS px, which is **not** a `juce::Font` height (that is
  ascent+descent). Use `labelFontHeightForCssPx` / `monoFontHeightForCssPx`; passing a spec number
  straight to `labelFont()` renders visibly small. Letter-spacing is absolute pixels JUCE has no
  setting for - `drawTrackedText` draws glyph-by-glyph, and the LCD's 9.6 px-per-character budget
  only holds with the .10em tracking applied.

**Program section.** `ProgramHeader` spans the content area and narrows its `hitTest` to the program
window plus SAVE/DELETE. Clicking anywhere in the *window* opens the dropdown, dressed by
`Chorus60MenuLookAndFeel` (ported from TapeRot, retinted to this LCD, red accent pip) at the window's
own width. The name cell shows `NN NAME`, plus a trailing ` *` while the loaded Program is edited,
and a chevron affordance at the right - drawn, not baked, because it is hidden during name entry and
during a parameter readout. While a control is dragged the cell shows `NAME: VALUE UNIT` in `#FFD9A0`,
centred, reverting 900 ms after release. **The caller guards that on the control's own drag state**
(`Chorus60EditorContent::attachReadout`): a `SliderAttachment` also fires when a Program is applied
and on every host automation step, and without the guard the display latches onto whichever parameter
was written last and flickers for the length of a song.

`engine1`/`engine2` are excluded from the modified-since-load check in `ProgramManager`. They are
stored in a Program, but they are the panel's pager and its bypass, hit mid-performance - counting a
press as an edit meant merely bypassing the plugin lit SAVE and claimed unsaved work.

`design/CHORUS60-GUI-SPEC.md` is the authoritative pixel spec, `design/CHORUS60-BUILD-HANDOFF.md` the
asset contract, and `design/CHORUS60-HANDOFF-README.md` carries deltas not yet folded into the spec
(the LCD chevron is one). Read them before touching GUI code - and **measure the plate rather than
trusting a coordinate**: the spec's section 5 puts the LCD window at x 571, the plate has it at 593,
and the two numbers the character budget rests on (a 59 px bank cell, a 352 px glyph run) are the ones
both agree on.

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

- **DSP**: every stage has real, functioning processing - no stubs, all grounded in
  `design/BBD-TECHNICAL-NOTES.md`'s description of the real circuit. Filter cutoffs and
  character-parameter ranges are a first, technically-reasoned pass rather than a tuned one - see
  `BUILDING.md`'s DSP tuning note. Build, load, listen, adjust. `auval` and
  `pluginval --strictness-level 8` both pass on AU and VST3.
- **GUI**: conformant to revision 2 and verified against it. The composite was captured from the
  Standalone and every region the build paints over the plate accounted for against the handoff's
  section 1.2 list - no double-prints, nothing straying onto baked furniture. The OFF state was
  measured rather than eyeballed (0.500 in the three boxes, 1.000 elsewhere, pointers unmoved), and
  the Rate pointer lands on its printed ticks to 0.20 degrees at minimum and 0.00 at maximum.
- **One release blocker**, tracked in `prompts/PROMPTS.md`: the @2x knob filmstrips are upsampled
  placeholders. They need Ø168 / Ø136 rendered from the original knob source, which is not in this
  project. The @1x strips ship meanwhile and are final; `Chorus60Theme::FilmstripSheet` carries the
  sheet geometry as data so the real sheets drop in without a code change.
- **Schema 3 is a hard break.** Session restore and the user-Program load path both discard state
  whose schema attribute is not current, so any `.chorus60program` written before this revision will
  no longer load - the `image*` rename and the Rate widening make a v2 file unreadable as v3. Nothing
  has shipped, so this is deliberate, but an existing user Program has to be re-saved.
- **Program bank**: 9 curated programs, `01 EIGHTY-TWO` default (spec section 11 confirms factory
  index 0; the `07 WIDE ENSEMBLE` that appears throughout the spec is an illustrative LCD string, not
  a bank entry). Values are structurally verified - ranges, the both-engines invariant, and a
  per-Program round trip asserting every stored value survives its parameter's mapping unchanged -
  but the bank has not had a by-ear pass.
- **Decay and Density** are intentionally automation-only parameters with no panel control.
