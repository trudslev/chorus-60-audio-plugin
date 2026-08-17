# CLAUDE.md

This file provides guidance to Claude Code when working with code in this repository.

CHORUS-60 is its own independent repo and does not depend on `../taperot/` or `../gatecrasher/` at
runtime - it's a sibling casting under the shared [Neon Foundry](../BRAND.md) umbrella, read here
purely as a structural reference (JUCE/CMake setup, APVTS conventions, DSP folder organization,
Tests/ approach, BUILDING.md shape, and - for Gatecrasher specifically - the Program storage
architecture and several GUI components ported directly rather than redesigned). Read `../BRAND.md`
first for the cross-plugin design system (naming, "Program" not "Preset", the one-accent-color rule,
component grammar), then this file for CHORUS-60's own conventions and status.
`design/GUI-SPEC.md` is the authoritative source for exact GUI pixel/asset detail and the sections
ported from Gatecrasher. Bundles no longer ship a `design/CLAUDE.md`; a repo-owned filename inside a
handoff reads as authority while being a snapshot of whatever the designers held when they cut it.

**The BBD technical notes do not exist anywhere, and the search is closed.** `design/BBD-TECHNICAL-NOTES.md`
was a 0-byte file from the initial commit until it was removed on 2026-08-12. Five DSP headers cite
it — `BBDDelayLine.h`, `CharacterStage.h`, `ModulationEngine.h/.cpp`, `StereoDecorrelationStage.h` —
and those citations now say "the missing BBD notes" rather than naming a filename that resolves to
nothing.

**Where it went, so nobody searches again.** The Chorus-60 designer searched their whole project on
2026-08-12: no notes, no addendum, no draft, and the only BBD string anywhere is a status-line label.
It was never on that side. Git says why — `design/BBD-TECHNICAL-NOTES-ADDENDUM.md` arrived in
`dc43d50` (2026-08-06), *this* repo, in the same commit that reworked the modulation engine. The
addendum was written here, quoting an original that whoever wrote it had open at the time and never
committed. Two days after the initial commit, in a session that left no other trace.

**What survives is enough to work from, and it is not the addendum alone.** The addendum corrects one
claim — I+II is a third fast/narrow/mono mode (~9.75 Hz, ~3.3–3.7 ms, no phase inversion), not a sum
of I and II — and cites its own source, which is public and readable:
<https://github.com/pendragon-andyh/Juno60/blob/master/Chorus/README.md>. That URL is the nearest
thing to ground truth this casting has. The five topics the headers cite (nonlinearities, tiny
imperfections, LFO shapes, the L/R difference, the bucket-brigade model) are **not** in the addendum,
which is why they were not repointed at it.

**A reconstruction was offered and declined.** The designer proposed writing a fresh
`BBD-CIRCUIT-NOTES.md`, dated and labelled as authored-now rather than recovered. It was the right
offer to make and the right one to refuse: a document written today from the addendum plus the public
source adds no information over reading those two directly, and once it sits in `design/` under a
plausible name the next person reads it as the thing the headers cite. **A confident reconstruction
of a lost source is worse than an acknowledged gap** — the gap is legible, the reconstruction is not.

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
                   │                                        CharacterStage.nextDriftMs ┤ (per sample)
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
disengaged, so re-engaging it never phase-jumps. Drift is a single slow-moving value read **per
sample** and added into the tap-position calculation before `BBDDelayLine` is read - it has to
perturb the actual delay time, not just color the audio afterward, per the technical notes'
"tiny delay jitter" description.

**This paragraph said "once per block (not per-sample - it retargets on a ~0.6s cycle, far slower
than the audio rate)" until 2026-08-15, and the parenthesis is the part worth keeping as a warning.**
Every word of it was true and none of it was the relevant rate: the *retarget* fires every 0.6 s, but
the *smoother it retargets* ramps over 0.3 s continuously, and reading a continuous ramp once per
block quantises it to the host's buffer size. That made the output depend on the buffer size, which
is the fourth member of the suite's per-block-stepped family (TapeRot's `genSmoothed`, its
`transportGateSmoothed`, Reflect-84's `LfoBank`). A correct sentence carried a wrong design.

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
I/II/I+II placeholder and the 16-name list `design/GUI-SPEC.md` section 9 suggests - the
shipped bank is authoritative over that suggestion. `01 EIGHTY-TWO` is the default on instantiation.

Its **core invariant**, enforced by `Tests/FactoryProgramsTests.cpp`: every program must sound
musically correct under ANY engine combination, not just the one it was authored in. The I/II
latches are front-panel controls a player hits mid-performance, so every program carries real,
considered Rate/Depth values for BOTH engines even when only one ships engaged - engaging an idle
engine must never produce silence, a stale value from the previous program, or a combination nobody
chose. The genuinely global parameters (Drift, Saturation, Noise, Mix, Output Trim) apply unchanged
across every combination; every configuration
carries its own complete set, so switching pages never inherits a value from the page before it.

### `CharacterStage`'s generator — four defects in one member, closed 2026-08-15

**This casting was the worst of the six at the bug sweep's baseline — nine failing assertions — and
it was one member.** `CharacterStage` held a single default-constructed `juce::Random`, and every one
of those nine traces back to it. Worth reading before touching that class, because the four defects
are genuinely different from each other and only two of them are about determinism.

| | The defect | What it broke |
|---|---|---|
| 1 | **Clock-seeded.** `juce::Random`'s default constructor seeds from the system clock | Two instances of the plugin were different instruments |
| 2 | **Not restored.** Neither `prepare` nor `reset` re-seeded it | A second render of ONE instance continued the first's stream, so the processor was not reproducible against itself even warmed with NOISE at zero |
| 3 | **Shared across consumers.** Drift retarget, wobble retarget and hiss all drew from it | The value a retarget got depended on how many hiss draws preceded it — a buffer-size question |
| 4 | **Channel-major hiss.** One stream feeding a `for ch { for i }` loop | The hiss landing on a given (channel, sample) sat at position `ch * numSamples + i`, which moves with the block size |

1 and 2 are closed by seeding in `reset()`; 3 and 4 by splitting into `driftRandom`, `wobbleRandom`
and a `hissRandom` per channel. The reasoning for each is in `CharacterStage.h` and `.cpp` beside the
code.

**Why 2 hid the other eight failures.** A processor that cannot reproduce itself makes every
comparison unreadable, so Chorus-60's block-size rows (0.733 / 0.725 / 0.340 / 0.704) and its
offline-against-real-time row (0.497) were measuring non-determinism rather than what they claimed.
They were never four defects and a fifth — they were **one unknown reported five times**. Offline
against real-time turned out to be sample-exact and was never a defect at all.

**And 3 is why the drift fix looked like it had failed.** Making drift per-sample (the family fix
above) took the 128 row from 0.004488230 to 0.000176609 and the 2048 row from 0.086103246 to
0.028221250 — but left 511 at 0.165607147, essentially unmoved. That was the evidence for a second
contributor, and a stage bisection found it in one run where reading for a candidate would not have:
restoring drift on top of an otherwise neutral wet path took the divergence from 0.000354871 to
0.201515645, while the LFO arm before it changed nothing.

### Silence in, silence out — a DECLARED property

**Measured 2026-08-14, not inferred.** Silence in, NOISE at 100 %:

| State | Output |
|---|---|
| **Neither engine latched** (the panel's OFF / BYPASS - SETTINGS RETAINED) | **peak 0.000000000 — silent** |
| One engine latched | peak 0.0089, −41.0 dB |

**So this casting generates a noise floor when engaged, and nothing at all when bypassed**, and both
halves of that are deliberate. `CharacterStage` is where the noise lives, and `processBlock`'s
disengaged branch copies `dryBuffer` straight to the output rather than running it — a true bypass,
not a wet path muted to silence.

**The question that settled it was not "is the noise floor deliberate".** It was whether the floor
runs while disengaged: a chorus emitting noise in its OFF state is a defect whatever the noise is
for, and a chorus that goes quiet has simply made a character choice. It goes quiet.

Written down because a suite-wide sweep found that Chorus-60 never falls silent at defaults and had
no way to tell whether that was intended — only TapeRot's generating was documented anywhere. This is
the answer stated in advance rather than rediscovered, and `Tests/NumericalRobustnessTests.cpp`
asserts it so the bypass cannot start emitting without a test failing.

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

### THE PLATE'S PRINTED LAYER — what it carried, and where each must now draw

**Written 2026-08-17, while BOTH plates are still in the tree, because this is the last moment the
reference exists.** `design/assets/chorus60-background-plate@2x.png` is the superseded plate and
`@3x.png` is the new one; once the old one goes, nothing states what it used to print.

**The failure mode INVERTED with the asset format and the new one is silent.** While the printed
layer was baked, the hazard was *double-printing* — a runtime draw landing on top of baked ink at a
one-pixel offset, which this file warns about in three places and which is **visible**. The new plate
carries the fascia gradient, the CHORUS badge and the box frames and nothing else, so the same
element now fails by being **absent**, and the panel merely looks slightly emptier than the render.

So the check the body layout needs is a **completeness** one, not a duplication one: enumerate what
the old plate printed and assert each has a drawing site. A forward check passes the moment
*something* is drawn; only comparing against what was there catches a subset — the same shape as the
icon set comparison, where a delivered directory smaller than the one it replaced passed every arm
that was written.

**Measured 2026-08-17: `drawTickRing`, `scaleNumeral` and `numeralRadius` return no hits anywhere in
`Source/`.** Not one tick, numeral or unit has a drawing site today. The printed layer is not
partially migrated; it is entirely unbuilt.

| The old plate printed | Count | Drawing site today |
|---|---|---|
| Tick rings — major and minor | 9 knobs | **none** |
| Scale numerals | 9 knobs | **none** |
| Units, in the arc's bottom gap | 6 of 9 | **none** |
| Global knob labels | 5 | **none** |
| Mod slot labels | 4 | `ModSlotLabels` — already runtime, the one part that was |
| Group headings — CHARACTER, OUTPUT | 2 | **none** (MOD ENGINE's is `paintOverChildren`) |
| Model line | 1 | **none** |
| Switch's printed STEREO / MONO | 2 | **none** — §5 puts them right of the sprite |
| PROGRAM / IN / OUT captions | 3 | the shared header part draws these now |
| Wordmark | 1 | **stays baked** — it is the CHORUS badge, §1 |
| Wells for scope, LCD, two meters | 4 | box frames stay baked; the wells' contents are runtime |

**Nothing in the "none" rows will fail a build, a test or a glance at a diff.** That is the whole
reason the table is here rather than left to the spec's element list — the spec says what the panel
should show, and only this says what stopped being drawn for it.

**"The plate is the single source of truth for where a mark sits" is now an OBITUARY, not
guidance.** That sentence appears below and was true; the thing it names is being deleted, and the
mark positions exist nowhere else in this repo. So **§3.1 is the only authority for them, not a
reference to check authoring against** — there is nothing left to check against.

**RATE is what makes that load-bearing.** Its fractions are catalogue §10's and are **not derivable
by even spacing**, so inferring them produces a plausible ring that nothing catches: the pointer
lands on marks, the marks look deliberate, and every value between them is wrong. Same trap as
Reflect-84's DAMPING HF, which *gained* a minor at .8333 that no amount of reasoning from the
dropped numerals would have produced. Author every mark from §3.1; infer none of them.

`PanelChrome` and `WordmarkComponent` are **deleted**, not adapted - everything they drew is
silkscreen now. So is `KnobValueLabel`, the standing value under every knob, and the drag-time popup:
revision 2 removed all standing readouts, and the LCD is the only numeric display on the panel. The
knob tick ring went the same way: revision 1 drew it at even angular spacing, and the plate now prints
every tick at its *labelled* value, which on the skewed Rate knob is not evenly spaced. **There is no
mark table in the code** - the plate is the single source of truth for where a mark sits.

**SUPERSEDED BY `682ac84` — the paragraph below describes the coordinate system this panel used
until 2026-08-17, and it gets rewritten with the body layout rather than now.** Coordinates are the
CANVAS's; the content component sits at (0, 0) at full size; `borderInset` and `contentWidth/Height`
are gone. It is left standing deliberately: correcting a comment ahead of the work it describes
documents intent rather than what was done, which is the failure this suite records separately from
a stale comment. Read it as a deferral, not an oversight.

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
centred, reverting 900 ms after release — `nf::describeParameter` and `nf::ReadoutFormat::revertMs`
from `neon-foundry-core`.

**`Chorus60Theme::formatParameterValue` is gone, and its four rules moved onto the parameters.** It
formatted by switching on the parameter's *label* — `Hz` to two places, `%` through `roundToInt`,
`ms` to one, `dB` with an explicit sign — which is a second formatting convention sitting beside the
parameter's own. That is exactly the arrangement that lets a panel and a host's automation lane
print the same control two different ways. All four are `stringFromValueFunction`s on the four
shared attribute sets in `Parameters.h` now; the output is identical and the host agrees.

`Chorus60Theme::readoutFormat()` holds the spelling, **not `ProgramHeader`** — `ProgramHeader.h`
reaches `PluginProcessor.h`, which needs `JucePlugin_*` macros that exist only in the plugin target,
so a test reading the format from there cannot link, and a test declaring its own copy would assert
against itself. `ValueCase::asAuthored`: the IMAGE switch's MONO/STEREO already arrive upper-case
from its own `stringFromValue`, which is where that decision belongs. **The caller guards that on the control's own drag state**
(`Chorus60EditorContent::attachReadout`): a `SliderAttachment` also fires when a Program is applied
and on every host automation step, and without the guard the display latches onto whichever parameter
was written last and flickers for the length of a song.

`engine1`/`engine2` are excluded from the modified-since-load check in `ProgramManager`
(`isPerformanceLatch`, passed to `nf::ParameterSnapshot::differsFrom` as its exclusion predicate).
They are stored in a Program, but they are the panel's pager and its bypass, hit mid-performance -
counting a press as an edit meant merely bypassing the plugin lit SAVE and claimed unsaved work.

**That is the lighter of two different tools and they get confused.** Excluding a parameter from the
dirty check leaves it stored and recalled, so Programs stay reproducible; excluding it from
*storage* is the heavier one, and is only correct when the Program cannot recall a state in which
that parameter is audible. Chorus-60 needs the light one; Fifth Member's Cross-Feed is the case that
needed the heavy test and failed it.

**The bank on disk, the dirty flag and Program identity all come from `neon-foundry-core`**, pinned
at `v1.0.0` and declared *after* `FetchContent_MakeAvailable(JUCE)` — core links `juce::juce_core`
and refuses to fetch its own, and two JUCE trees in one build link two `juce_core` builds into one
binary. It is linked into both `Chorus60` and `Chorus60Tests`, because `ProgramManager.cpp` is
compiled into both.

What a Program *contains* stays here: the whole APVTS state plus the schema version. Two things
changed with the move — the empty-name fallback is **`TAKE n`**, not `NEW PROGRAM`, and the dirty
baseline is **keyed by parameter ID and guarded by a `SpinLock`** where it was a positional
`std::vector<float>` with neither. The old comment claimed every writer ran on the message thread;
`setStateInformation` carries no such guarantee, and the GUI polls the flag while it runs.

**The user-Programs path takes company and product as arguments, never a shared default** — this is
the casting whose hand-synced copy of that path drifted to "Tanis" after `COMPANY_NAME` changed,
quietly pointing saved Programs at a dead directory. A default inside core would reintroduce that in
one place for all six.

**The header band is 34px at y 32, and the plate is what settles it.** LCD well, both Program
buttons and both meter windows share one height. The row was 28-29 here, with the LCD's border box a
pixel larger than its own neighbours - the drift the suite audit found in four castings.

**The spec's table disagreed with the plate again, and the plate won again.** This file already
records one such disagreement (the spec's x 571 against the plate's 593). In this revision the spec
gives the window as 451 wide at x 519 where the plate has **414 at x 557** — only the right edge
agrees (519 + 451 = 970; the plate's name cell ends at 969.5), so the two describe the same
right-hand edge and different left-hand ones. The whole LCD moved **36px left**: tag cell 594 → 558,
name cell 654 → 618, both by exactly 36. The name cell stayed 352 wide, so the 36-character budget
is untouched.

**The plate now bakes both button faces, and the build draws only the legends.** That is §13's
split: each of the four legends lights independently, so a baked legend would freeze one state's
lighting into the bitmap — but the face has no state to freeze, so it belongs in the plate with the
rest of the static furniture. **Nothing in `ProgramHeader` fills or borders these buttons**; doing so
would paint a live control over a baked copy of itself, which is the bug this casting's own notes
keep naming.

The buttons were 43 × 28 and 55 × 28 — two different widths. They are 70 × 34 each now, and the
second legend is what makes them equal rather than what forced them apart: each is sized by its
longest word, and DELETE and CANCEL are both six characters.

**The face went pale → dark, and that follows from the treatment.** A pale face is the one that
requires the lamp-beside-legend form; backlit legends need somewhere brighter to go. Nine colour
constants went with it, including a disabled label that had itself just been rescued from `#8B9297`
at **1.42:1**. The disabled state stopped existing rather than getting a better grey.

The bloom is deliberately **warm** against the cool-neutral face — a warm bloom reads as backlit
where a neutral one reads as merely brighter ink, which is the distinction BRAND.md draws. It is not
the accent: the engine reds and yellows stay with I / II / I+II and appear nowhere in the header.

`design/GUI-SPEC.md` is the authoritative pixel spec, `design/CHORUS60-BUILD-HANDOFF.md` the
asset contract, and `design/README.md` carries deltas not yet folded into the spec
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

### The Program list's group caption

**Sized from its own type plus padding, never derived from the row height.** The construction is
`nf::captionHeight (font, topPadding, bottomPadding)` — 3px above and 4px below, the suite's adopted
default — and it comes out **18px** here, from a nominal 11px built from a JUCE height rather than through `withPointHeight`.

**The construction is the rule, not the number.** Writing 18 as a literal would break silently at
the first change of font, size or font construction, which is a change nobody would think to check a
caption against. It is also how this caption came to inherit JUCE's `rowHeight + rowHeight / 2` in
the first place — a caption half again *taller* than a row, which is a menu convention rather than a
panel one.

**The 18 is not a divergence to correct.** Predicting 19 for all four castings and measuring 18
here is what surfaced the type-scale finding: this casting owns a `monoFontHeightForCssPx`
converter and the menu type bypasses it, so the same nominal constant renders smaller. That is a
question about the whole type scale, recorded in the root `CLAUDE.md`, not about captions.

### Case belongs at the source

`nf::ReadoutFormat::ValueCase` is deleted from core (2026-08-13). **A panel label reads in caps
because it is authored in caps in `Parameters.h`**, not because the readout upper-cased it on the
way out — the panel and the host's automation lane read the same parameter, so any re-casing in
between makes one of them lie about the other. Every parameter name here is authored in caps for
that reason, and `Tests/ReadoutConformanceTests.cpp` asserts it off `getName()`.

The rule is in `../BRAND.md` beside the unit rule; the suite-wide record is in the root
`../CLAUDE.md`. **The choice strings are deliberately NOT all caps** — this casting prints its
values as authored, and the rule is that case is decided at the source, not that everything is caps.

## Status

- **DSP**: every stage has real, functioning processing - no stubs, all grounded in
  the designers' BBD technical notes on the real circuit (not in the repo — see the note at the top of `CLAUDE.md`). Filter cutoffs and
  character-parameter ranges are a first, technically-reasoned pass rather than a tuned one - see
  `BUILDING.md`'s DSP tuning note. Build, load, listen, adjust. `auval` and
  `pluginval --strictness-level 8` both pass on AU and VST3.
- **GUI**: conformant to revision 2 and verified against it. The composite was captured from the
  Standalone and every region the build paints over the plate accounted for against the handoff's
  section 1.2 list - no double-prints, nothing straying onto baked furniture. The OFF state was
  measured rather than eyeballed (0.500 in the three boxes, 1.000 elsewhere, pointers unmoved), and
  the Rate pointer lands on its printed ticks to 0.20 degrees at minimum and 0.00 at maximum.
- **Knob filmstrips are complete and reproducible.** All four sheets ship rendered natively at their
  own size, and the generator ships with them as `design/tools/render-knob-filmstrips-chorus60.mjs`,
  so the art can be regenerated at any diameter rather than being a binary nobody can rebuild. The
  earlier release blocker is closed.
- **Cap diameter is not frame pitch, and `FilmstripSheet::capFraction` is what keeps them apart.**
  The generator pads each frame so the cast shadow fades to zero inside it - measured border alpha
  <= 2 on all four sheets - so the mod cap is Ø84 in a 112 box (0.750) and the global Ø68 in a 92 box
  (0.739). Those two ratios differ for a real reason, not rounding. `KnobFilmstripComponent` draws
  into `diameter / capFraction` so the CAP lands at section 8's size; size from the pitch instead and
  every control comes out about 25% small. The generator's own header says it: "position knobs from
  the CAP centre, not the frame box."
- **Program bank**: 9 curated programs, `01 EIGHTY-TWO` default (spec section 11 confirms factory
  index 0; the `07 WIDE ENSEMBLE` that appears throughout the spec is an illustrative LCD string, not
  a bank entry). Values are structurally verified - ranges, the both-engines invariant, and a
  per-Program round trip asserting every stored value survives its parameter's mapping unchanged -
  but the bank has not had a by-ear pass.
- **Decay and Density** are intentionally automation-only parameters with no panel control.
