# CHORUS-60 CH-60 — GUI Implementation Spec

**Build from this file.** It is the contract for everything the panel looks like and does. One
companion carries the rest: **which assets ship as bitmaps vs. are drawn at runtime, and where the
files live — see `CHORUS60-BUILD-HANDOFF.md`.** Nothing else in the bundle is a contract; the
`README.md` is orientation and the `.dc.html` prototypes are reference.

Brand-level rules (wordmark, palette roles, the legibility floor, the Program-button treatment this
panel implements) live in `BRAND.md` **in the repo** — deliberately not copied into this bundle, so
there is one authority and it is the current one.

**Revision 2 — density, printed scales, LCD, contrast, OFF state.**
Everything in the identity is unchanged: dark synth-panel fascia, blue CHORUS stripe, three coloured
engine buttons, silkscreen wordmark, delay-modulation scope, red accent. This revision changes
*density and legibility only*.

Panel: **1280 × 775 px** at 1× (1282 × 777 including the 1 px outer border, which is what the
exported plate measures). Down from 1400 × 632 — narrower and taller, so nine knobs and four buttons
sit at instrument density rather than diagram density.

**Asset contract:** `CHORUS60-BUILD-HANDOFF.md` states what is baked into the background plate and
what the build draws at runtime. Read it before wiring anything up — as of this revision the plate
**does** carry glyphs (printed scales, static labels), which was not true of the previous plate.

Renders:

| File | State |
|---|---|
| `assets/chorus60-background-plate@1x.png` | 1282 × 777 — plate only, no knob sprites, no lamps, no LCD text |
| `assets/chorus60-background-plate@2x.png` | 2564 × 1554 — same, @2x |
| `assets/chorus60-page-i@2x.png` | Engine I engaged |
| `assets/chorus60-page-ii@2x.png` | Engine II engaged |
| `assets/chorus60-page-i-plus-ii@2x.png` | Both engaged |
| `assets/chorus60-page-off@2x.png` | **OFF / bypass — new treatment** |

Product icon: **unchanged.** `Chorus-60 Icon.dc.html` and the 16–1024 icon ladder in
`assets/icon/` are not affected by this revision and must not be regenerated.

---

## 1. What changed in this revision

1. **Canvas 1400 × 632 → 1280 × 775.** Controls grew; spacing tightened.
2. **Knobs enlarged.** Mod Engine Ø58 → **Ø84**; global Ø48 → **Ø68**. Filmstrips **re-rendered at
   the new diameters** — see §6.
3. **Printed scales added to every knob** (ticks + numerals + unit), §7.
4. **Standing value readouts removed.** No live numbers anywhere on the fascia; the LCD is the only
   numeric display.
5. **LCD widened** to 376 px of name field, 352 px of glyph run — 36 characters at its font size, §5.
6. **FACT / USER moved to the segment face** — it is display text, so it is set like display text.
7. **Palette shifted up one step** for contrast, §2.
7b. **IMAGE switch renamed** — the MONO/STEREO parameters become `image1`/`image2`/`imageB`, §7.2.
7c. **Button column re-spaced** to the JN-80's even rhythm; buttons 120 → **132**, §4.
8. **OFF state**: 0.42 → **0.50** opacity, wind-to-zero **removed**, `SETTINGS RETAINED` caption
   **removed**, §9.

---

## 2. Palette

Measured against the group panel field `#131517` unless stated.

| Role | Hex | Contrast | Notes |
|---|---|---|---|
| Heading / engaged label | `#E6EBEE` | 12.9 : 1 | Wordmark, `DELAY MODULATION`, engaged button letters |
| **Control label / printed scale numeral** | `#A5ADB2` | **8.04 : 1** | Functional text — knob names, tick numerals, `STEREO`/`MONO`, inactive button letters, `BBD CHORUS PROCESSOR` |
| **Caption / tertiary** | `#8A9196` | **5.73 : 1** | Units under the scale, `PROGRAM`/`IN`/`OUT` captions, `MODEL CH-60 · STEREO`, scope status text, box titles when bypassed, footer |
| Scope annotations (`DLY MOD`, `+ MAX`, `- MAX`) | `#9BA3A8` | 5.6 : 1 on the scope well (`#06080A`–`#0B0F11`) | Drawn opaque — **no alpha**. The old `rgba(160,178,186,.55)` measured 3.11 : 1 and is gone |
| LCD text (program name, FACT/USER, IN/OUT) | `#DFE6EA` | 14.6 : 1 on `#07090A` | + `text-shadow: 0 0 8px rgba(200,220,230,.25)` |
| LCD parameter readout | `#FFD9A0` | 11.7 : 1 on `#07090A` | Only while a control is moving |
| Accent (scope trace, engine lamps) | `#FF2B1C` | — | Red, unchanged |
| Blue stripe | `#2F6DB8` → `#1F5798` | — | Unchanged hue |
| Panel field | `#141618` → `#0E1012` → `#0A0C0D` | — | Unchanged |
| Group box | `#131517` → `#0C0E10`, 1 px `#000` border, 1 px `rgba(255,255,255,.05)` inset top | — | Unchanged |

Retired values, for the build's find-and-replace: `#8A9196` as a *label* colour → `#A5ADB2`;
`#7B8287` → `#8A9196`; `#5F666B` → `#8A9196`; `#5A6165` → `#8A9196`; `#C6CED3` (value readouts) →
deleted with the readouts.

Rule of thumb: **everything shifted up one step.** The old label grey is the new flavour grey.

## 3. Typography

- Labels / headings: **Barlow Condensed** — 600 for labels and captions, 700 for group headings and
  button letters.
- Display: **Share Tech Mono** — the segment face. Everything *inside a display* is set in it:
  program name, `FACT` / `USER`, IN/OUT meters, the parameter readout, and the printed scale
  numerals (which read as engraved display text, matching Gatecrasher).

| Role | Face | Size | Tracking |
|---|---|---|---|
| Wordmark | Librestile Extended Bold | 34 px | .02em |
| Section heading (`DELAY MODULATION`) | Barlow Condensed 700 | 11 px | .28em |
| Group heading (`MOD ENGINE I`, `CHARACTER`, `OUTPUT`) | Barlow Condensed 600 | 11 px | .28em |
| **Control label** (`DELAY CENTER I`) | Barlow Condensed 600 | **12 px** | .18em |
| **Printed scale numeral** | Share Tech Mono | **12 px** | 0 |
| **Scale unit** (`Hz`, `%`, `ms`, `dB`) | Barlow Condensed 600 | **11 px** | .16em |
| Caption (`PROGRAM`, `IN`, `OUT`) | Barlow Condensed 600 | 9 px | .24em |
| LCD program name / `FACT` / `USER` | Share Tech Mono | **15 px** | .10em |
| IN / OUT meters | Share Tech Mono | 13 px | .06em |
| Scope status (`ENGINE I`, `250 ms / DIV`) | Share Tech Mono | 11 px | .06em |
| Blue stripe (`CHORUS`) | Barlow Condensed 700 | **26 px** | .34em |
| Button letters `I` / `II` | Barlow Condensed 700 | 22 px | .06em |
| Button letter `OFF` | Barlow Condensed 700 | 18 px | .14em |
| Footer | Share Tech Mono | 10 px | .10em |

Minimum type size anywhere on the panel: 9 px, and only for the three all-caps captions.

## 4. Layout

Origin is the inside of the panel border. Panel content area 1280 × 775.

| Region | x | y | w | h |
|---|---|---|---|---|
| Header band | 0 | 0 | 1280 | 78 |
| Body | 0 | 78 | 1280 | 669 |
| Footer band | 0 | 747 | 1280 | 28 |
| Button column | 22 | 96 | **220** | 629 |
| — blue stripe (top) | 22 | 96 | 220 | **46** |
| — blue bar (bottom) | 22 | 701 | 220 | **24** |
| Vertical rule | 263 | 96 | 1 | 629 |
| Scope block (caption row + well) | 285 | 96 | 973 | 140 |
| — scope well | 285 | 116 | 975 | 120 |
| MOD ENGINE box | 285 | 252 | 973 | 240 |
| CHARACTER box | 285 | 508 | 567 | 218 |
| OUTPUT box | 868 | 508 | 390 | 218 |

**Header row — the five-element band.** LCD well, SAVE, DELETE, IN and OUT all sit at **y 31 with an
outer height of exactly 34 px**, one shared baseline at y 65. The row grew from 28 to 34 px in the
suite-wide header pass; the 6 px came out of the header band's own padding (16 / 14 → 14 / 12) so the
band stays 78 px and **no body coordinate moved**. Do not take the height out of the LCD instead.

| Element | x | w | h |
|---|---|---|---|
| LCD well | 519 | 451 | 34 |
| SAVE / STORE button | 976 | 70 | 34 |
| DELETE / CANCEL button | 1052 | 70 | 34 |
| IN meter | 1138 | 56 | 34 |
| OUT meter | 1202 | 56 | 34 |

Captions (`PROGRAM`, `IN`, `OUT`) sit 6 px above the row at y 13. Meters widened 54 → 56 px so the
border sits inside the stated box, matching the well and the buttons.

Body padding 18 / 22 / 22; column gap 21 either side of the rule; 16 px between stacked blocks;
16 px between CHARACTER and OUTPUT. Group boxes: 1 px `#000` border, 8 / 16–18 / 12 padding,
heading row separated by a 1 px `rgba(0,0,0,.6)` rule with a `rgba(255,255,255,.05)` highlight under it.

**Engine buttons** — **132 × 132**, 5 px radius, at x 26:

| Button | y | Fill |
|---|---|---|
| II | 183 | `linear-gradient(160deg,#E5A021,#C07908)` |
| I | 356 | `linear-gradient(160deg,#EAD681,#D0B857)` |
| OFF | 528 | `linear-gradient(160deg,#EAECEC,#C9CDCF)` |

Lamp: 15 px circle, 16 px to the right of the button, then 12 px to the letter. Pressed state
translates the button +3 px in y for 110 ms.

**Column spacing follows the JN-80.** On the hardware the four vertical gaps — stripe-to-first-button,
the two gaps between buttons, and last-button-to-bottom-bar — are all roughly equal, at about 0.3 ×
the button height; the column reads as a packed block of switches, not three buttons floating in a
field. The panel reproduces that: buttons are sized so all four gaps come out at **41 px** against a
132 px button (0.31 ×), filling the column edge to edge. An earlier draft centred a 120 px stack,
which left the top and bottom gaps at 2.3 × the inter-button gap and made the column read far
sparser than the rest of the panel. **Do not centre the stack — distribute it.**

## 5. Program LCD

| | |
|---|---|
| Window | **451 × 34 at x 519, y 31** — one inset well holding the bank cell and the name field. Height is border-box: 34 px **including** the 1 px border, so the well's outer height equals the buttons' and the meters' exactly |
| Bank cell | 59 px wide at x 520, right-hand 1 px `#2A3035` divider |
| **Name field** | **390 px at x 579**, 12 px left / 26 px right padding (the right pad is the chevron gutter) → **352 px of glyph run** |
| Face / size | Share Tech Mono 15 px, .10em tracking → **9.6 px per character** |
| **Character budget** | **36 characters** at 15 px — 352 / 9.6 |
| **User-name cap** | **31 characters** — budget 36 less `NN ` (3) and the ` *` dirty marker (2) |
| Well | `#07090A`, 1 px `#363C41`, `0 2px 6px rgba(0,0,0,.9)` inset |

`FACT` / `USER` is set in **the same face, size, tracking and colour as the program name** — it is
inside the display, so it is display text. It is no longer dimmed relative to the name.

Three contents, one field:

1. **Program** (idle) — `07 WIDE ENSEMBLE`, centred, `#DFE6EA`.
2. **Parameter readout** (while any control is moving) — left-aligned, `#FFD9A0`, format
   `NAME: VALUE UNIT`, e.g. `DECORRELATION I+II: 100 %` (25 characters — 240 px, comfortably inside
   the 352 px field with 112 px to spare). Held for 900 ms after release, then the program name
   returns. This is the panel's *only* live numeric display.
3. **Name entry** (after STORE is armed) — left-aligned typed text, **31-character limit**, blinking
   block caret. The bank cell reads `NAME` while typing, not `USER` — the Program is not in the user
   bank until the name is committed.

**The cap is 31 and may never fall below it.** The header-height change to 34 px did not touch the
name field's width, face, size or tracking, so the budget is still 36 and the cap is still 31; the
constants `LCD_BUDGET = 36` and `NAME_CAP = 31` are named in the prototype logic for that reason.
The build previously truncated typed names at 24 characters, which was below the published cap —
corrected to 31. Any future change to header height, font size, tracking or cell width must restate
the resulting budget and confirm it has not fallen.

## 6. Knob assets — **CHANGED, re-render required**

| Sheet | Cap Ø | Frame box | Frames | Sheet size | Used by |
|---|---|---|---|---|---|
| `assets/knob_mod_84px_128f.png` | **84 px** | **112** | 128 | 112 × 14336 | Rate, Depth, Delay Center, Decorrelation |
| `assets/knob_global_68px_128f.png` | **68 px** | **92** | 128 | 92 × 11776 | Drift, Saturation, Noise, Mix, Output Trim |
| `assets/knob_mod_168px_128f@2x.png` | **168 px** | **224** | 128 | 1792 × 3584 | as above, @2x — 8 × 16 grid |
| `assets/knob_global_136px_128f@2x.png` | **136 px** | **184** | 128 | 1472 × 2944 | as above, @2x — 8 × 16 grid |

**The frame box is larger than the cap** (ratio 0.75; global 0.739). The margin carries the cast
shadow to zero — do not crop it, and centre each knob on the **cap**, not the frame box.

All four sheets are rendered natively at their own size by a parametric generator taking a scale
factor S — nothing is scaled at draw time and nothing is upscaled. The Ø58 / Ø48 usages are retired.

**The cap look changed this revision**: lighter body, flatter falloff, knurl reaching further in.
This matches Gatecrasher, which shares the knob. It is a visible panel change, not a resolution swap
— reference renders predate it. The Ø128 files in `assets/` are retired outputs, not a source;
future re-renders go back to the generator. See §4 of `CHORUS60-BUILD-HANDOFF.md`.

Frame *n* (0…127) = `background-position: 0 −(n × frame box)px` for the @1x strips — that is
−112 n for mod and −92 n for global, **not** the cap diameter. The @2x sheets are 8 × 16 grids: frame
*n* at column *n* mod 8, row ⌊*n* / 8⌋. Frame 0 = −135°, frame 127 = +135°.
The knurl ring that used to sit around each knob is **removed** — the printed scale replaces it.

## 7. Printed scales

Every knob carries a printed scale: five ticks, five numerals, one unit. This is **functional text
at 7 : 1** (`#A5ADB2`), and it is the only at-rest value reference on the panel.

Geometry, in the knob cell's own coordinates:

| | Mod Engine cell | Global cell |
|---|---|---|
| Cell | 176 × 164 | 158 × 144 |
| Knob centre | (88, 82) | (79, 72) |
| Knob | Ø84 | Ø68 |
| Tick | 2 × 9 px, centred at r = 47 (spans r 42.5 → **51.5**) | 2 × 9 px, centred at r = 38 (spans r 33.5 → **42.5**) |
| Numeral | **6 px of clear air past the tick's outer end** — radius computed per label, see below | same rule, tick outer end r = 42.5 |
| Unit | centred, row at y 148 | centred, row at y 128 |
| Control label | below the cell, 6 px gap | below the cell, 6 px gap |

Ticks are drawn **at the labelled values, not at even angles**, and rotated to point at the knob
centre.

**Numerals are placed by their nearest glyph edge, not by their box centre.** A single ray radius for
every mark looks wrong: on the Rate knob it puts `0.05` almost touching its tick while `8` floats
twenty pixels clear, because a four-character label reaches much further along a diagonal ray than a
one-character label does along a horizontal one. Each numeral is instead positioned so its nearest
edge clears the tick's outer end by a constant **6 px**:

```
edge  = min( (len × 7.2) / 2 / |sin θ| ,  14 / 2 / |cos θ| )   // ray exit of the label box
r     = r_tick_outer + 6 + edge
centre = ( cx + r·sin θ , cy − r·cos θ )
```

7.2 px is the Share Tech Mono advance at 12 px; 14 px is the line box. The resulting radii on Rate
run 61 – 70 px, and every numeral reads at the same distance from its tick. Angle for a mark at sweep fraction *p*: `θ = −135° + 270° · p`.

### 7.1 Marks and tick angles

| Knob | Unit | Marks | Sweep fraction | Tick angle |
|---|---|---|---|---|
| **Rate** | Hz | 0.05 / 0.5 / 2 / 8 / 16 | 0 · .287 · .479 · .784 · 1 | **−135.0° · −57.5° · −5.7° · +76.7° · +135.0°** |
| **Depth** | % | 0 / 25 / 50 / 75 / 100 | 0 · .25 · .5 · .75 · 1 | −135° · −67.5° · 0° · +67.5° · +135° |
| **Delay Center** | ms | 2 / 5 / 8 / 11 / 14 | 0 · .25 · .5 · .75 · 1 | −135° · −67.5° · 0° · +67.5° · +135° |
| **Decorrelation** | % | 0 / 25 / 50 / 75 / 100 | 0 · .25 · .5 · .75 · 1 | −135° · −67.5° · 0° · +67.5° · +135° |
| **Drift / Saturation / Noise / Mix** | % | 0 / 25 / 50 / 75 / 100 | 0 · .25 · .5 · .75 · 1 | −135° · −67.5° · 0° · +67.5° · +135° |
| **Output Trim** | dB | −12 / −6 / 0 / +6 / +12 | 0 · .25 · .5 · .75 · 1 | −135° · −67.5° · 0° · +67.5° · +135° |

Rate is power-law skewed, so its numerals are visibly irregular — that is correct and is how skewed
hardware prints.

**Implement the taper as a single power law: skew 0.35 over 0.05 – 16 Hz** (a JUCE
`NormalisableRange` skew factor of 0.35, or any equivalent). That reproduces the five printed anchors
to within 0.09° across the sweep — about a twentieth of a pixel at the tick radius — so the printed
scale and the parameter agree by construction. **No lookup table and no piecewise interpolation are
required.** (The reference implementation interpolates piecewise between the anchors; that is an
artefact of the prototype, not a requirement.) The anchors the skew must land on:

```
0.05 Hz @ 0%   0.5 Hz @ 28.7%   2 Hz @ 47.9%   8 Hz @ 78.4%   16 Hz @ 100%
```

Units print **in the scale area** (small, caption grey, under the knob, above the control name) and
are never appended to the control name.

### 7.2 IMAGE switch

The MONO/STEREO toggle occupies a 132 × 164 cell in the same row: 34 × 68 track at y 48, 26 px thumb,
thumb travel 34 px. `STEREO` and `MONO` are printed **at the thumb centres** — label rows at y 58 and
y 92, giving centres of y 65 and y 99, exactly where the thumb sits in each position. Both are
right-aligned in a 44 px box at 12 px Share Tech Mono in `#A5ADB2` — the switch's printed scale, held
to the same rule as the knob ticks: **the print sits at the position it names.**

**The control is called IMAGE, and the parameter must be renamed to match.** The switch's two values
are MONO and STEREO, but those are its *positions*, printed beside the thumb — the same way
Gatecrasher's KEY SOURCE switch prints INTERNAL / SIDECHAIN. The control's name is what it does to
the signal. The build must therefore rename the parameters `mono1` / `mono2` / `monoB` →
**`image1` / `image2` / `imageB`**, with display names `IMAGE I` / `IMAGE II` / `IMAGE I+II` and
value strings `MONO` / `STEREO`, so the host's generic parameter list and the fascia agree.

**Only the control's name changes. The switch positions stay printed STEREO and MONO** — they are the
values, and they keep the wording the host reports. The rename is the name field alone:
`mono1 → image1`, display `MONO/STEREO I → IMAGE I`.

Panel and host must not be shipped divergent. If the rename is refused, the panel label reverts to
`MONO/STEREO I` and everything else — the printed positions, the geometry, the LCD readout format —
stays as specified. Label below: `IMAGE I` /
`IMAGE II` / `IMAGE I+II`.

## 8. Knob positions

Mod Engine row: four 176 px cells + the 132 px switch cell, 20 px gaps, centred in the box
(row width 916 px, first cell at x 314). Knob centres at **y 376**, x **402 · 598 · 794 · 990**;
switch cell at x 1098, centre x 1164.

CHARACTER: three 158 px cells, 22 px gaps, centred (first cell x 310). Knob centres at **y 620**,
x **389 · 569 · 749**.
OUTPUT: two 158 px cells, 22 px gaps, centred (first cell x 894). Knob centres at **y 620**,
x **973 · 1153**.

All rows are centred in their box, so these x values are derived — if the button column width ever
changes again, re-derive them rather than transcribing.

Drag: vertical, full range over 200 px, ×0.25 with Shift, double-click resets to default.
Mod Engine drags are normalised — they move the *sweep fraction*, so Rate drags evenly in
perceived pitch rather than in Hz.

## 9. OFF / bypass state — **CHANGED**

Pressing OFF (or releasing both engine buttons) puts the panel in bypass:

- The three control groups — MOD ENGINE, CHARACTER and OUTPUT, from each box's heading rule down —
  are multiplied by **0.50**. The header band, the scope, the button column and the footer are **not**
  dimmed: the engine buttons keep their full saturation, because on the hardware they are moulded
  plastic that never changes, and the LCD and meters stay legible. The fade is a **multiply, not an
  alpha blend toward the background colour**. The model is a lamp being switched off: multiplying by
  0.50 scales every pixel toward black and preserves the relative contrast inside the group, so the
  panel reads as *darker*. Blending toward the panel field instead washes the group toward a single
  flat grey and reads as fog laid over the fascia — wrong physics, wrong feel. (In CSS these coincide
  only because the backdrop is near-black; on any other backdrop the build must multiply.) Up from 0.42. 0.70 was tried and read as almost no
  change at all on a fascia this dark; 0.50 is an unmistakable power-down. The label grey lands near
  3.7 : 1 while bypassed, below the 4.5 floor the live panel holds. This is deliberate: nothing is
  adjustable in bypass and the panel is not meant to be read in this state — the printed values are
  there to be recognised, not consulted. Desaturating the fascia instead of dimming it was tried and
  rejected.
- **Pointers do not move.** The wind-to-zero animation is deleted. Real hardware doesn't move its
  knobs on bypass; the lamps just go out. Rotating pointers to zero depicted values that weren't
  current, which is why the panel used to have to print `SETTINGS RETAINED` to correct the
  impression.
- **`BYPASS · SETTINGS RETAINED` → `BYPASS`.** With the pointers no longer lying, the reassurance is
  unnecessary.
- Engine lamps swap to the `lamp-off` sprite. **This is the only sprite swap.**
- The engine button letters follow engagement, not bypass: engaged `#E6EBEE`, not engaged `#A5ADB2`.
  In bypass none is engaged, so `I` and `II` read `#A5ADB2` and `OFF` reads `#E6EBEE`. This is a state
  readout that already exists on the live panel — it is **not** a dimming effect, and it is the only
  per-element colour change in the OFF state.
- **No other element changes colour.** Group headings, control labels, printed scales and units are
  baked into the plate and cannot restyle; the multiply is what dims them. Earlier drafts of this
  section also dropped the headings to `#8A9196` — that instruction is withdrawn, and applying both
  would leave those elements darker than their neighbours.
- Scope keeps drawing the residual drift/noise floor.
- Scope status reads `ENGINE BYPASS`; footer reads `BBD 1024 STAGE · BYPASS · v1.0`.
- No colour filter, no grayscale, **no reflow**. Layout is identical to the live state.
- Pointer interaction is disabled on all knobs and the switch; the LCD, SAVE/DELETE and the engine
  buttons stay live.

Reference render: `assets/chorus60-page-off@2x.png`.

## 10. The paged MOD ENGINE box

Unchanged from revision 1, restated for completeness. **The physical I / II / OFF buttons are the
pager** — there is no tab strip and no page arrows.

| I | II | Page | Heading | Status |
|---|---|---|---|---|
| on | off | `I` | `MOD ENGINE I` | `ENGINE I ENGAGED` |
| off | on | `II` | `MOD ENGINE II` | `ENGINE II ENGAGED` |
| on | on | `I+II` | `MOD ENGINE I+II` | `BOTH ENGAGED · MONO BBD PAIR` |
| off | off | last page held | last page's heading | `BYPASS` |

Each page owns a full parameter set — Rate, Depth, Delay Center, Decorrelation, Image. Slot order is
fixed, so a page change slews each pointer from its old value to the new one over **380 ms**, using a
frame-rate-independent slew `k = 1 − 0.002^(dt/380ms)` on a single shared rAF loop per instance
(generation-guarded at `globalThis` level). A drag bypasses the slew and tracks 1 : 1.
Decorrelation is always live and adjustable, including on I+II where it is inaudible under MONO and
becomes meaningful when the Image switch is thrown to STEREO.

## 11. Parameters

| ID | Name | Range | Default (I / II / I+II) | Skew |
|---|---|---|---|---|
| `rate` | Rate | 0.05 – 16 Hz | 0.45 / 2.90 / 1.20 | **Power-law, anchors in §7.1** |
| `depth` | Depth | 0 – 100 % | 38 / 64 / 52 | linear |
| `center` | Delay Center | 2 – 14 ms | 5.6 / 4.2 / 6.4 | linear |
| `decorr` | Decorrelation | 0 – 100 % | 52 / 66 / 44 | linear |
| `image1` / `image2` / `imageB` | Image | `MONO` / `STEREO` | STEREO / STEREO / **MONO** | switch — renamed from `mono1`/`mono2`/`monoB`, see §7.2 |
| `drift` | Drift | 0 – 100 % | 22 | linear, global |
| `sat` | Saturation | 0 – 100 % | 30 | linear, global |
| `noise` | Noise | 0 – 100 % | 14 | linear, global |
| `mix` | Mix | 0 – 100 % | 50 | linear, global |
| `trim` | Output Trim | −12 – +12 dB | 0 | linear, global |

Rate's range was **0.05 – 8 Hz** in revision 1 and is now **0.05 – 16 Hz**, to match the printed
scale.

Default program on load: **factory index 0**. `07 WIDE ENSEMBLE` appears throughout this document and
in the renders purely as an example LCD string — it is not a bank entry and does not set the default.
Wherever a program name is shown here, read it as illustrative.

## 12. Unchanged

The build should keep all of the following exactly as delivered — none of it is affected by this
revision and none of it needs regenerating:

- **Product icon** — `assets/icon/` ladder (16 · 32 · 64 · 128 · 256 · 512 · 1024 + light-plate 512)
  and `Chorus-60 Icon.dc.html`.
- **Knob masters** — `knob_large_128px_128f.png`, `knob_small_128px_128f.png` (Ø128, the source the
  Ø84 / Ø68 sheets are rendered from).
- **Wordmark face** — `assets/LibrestileExtBold.ttf`.
- **Reference photography** — `jn80-chorus-reference.jpeg`, `gatecrasher-panel-reference.png`.
- ~~**Header state renders**~~ — superseded. `header-factory-program@3x.png`,
  `header-user-program@3x.png` and `header-name-entry@3x.png` show the retired 28 px row with pale
  buttons; **discard them**. Replaced by `reference/chorus60-header-naming@2x.png` and the six
  `program-*` sprites.

- Product icon and the full 16–1024 ladder in `assets/icon/`.
- Wordmark: Librestile Extended Bold, silkscreen treatment, `text-shadow: 0 1px 0 rgba(0,0,0,.9),
  0 0 1px rgba(230,235,238,.5)`.
- Scope construction: 8 × 6 grid, 250 ms/div, scrolling 2 s window, three-pass red trace
  (glow / halo / core), BBD noise floor behind it, playhead dot at the right edge.
- Factory-vs-user Program semantics: SAVE always creates a new Program and never overwrites; DELETE
  acts only on User Programs. **The buttons' faces and legends did change — see §13.**
- Blue stripe hue and its two-band placement above and below the button column.
- Fonts, group-box construction, footer contents.

## 13. Program buttons — **CHANGED**

Both Program buttons carry **two printed legends each, stacked, and never change their face**: SAVE
above STORE, DELETE above CANCEL — the resting function on top, what the button becomes during name
entry beneath it. Nothing relabels itself.

| | |
|---|---|
| Box | **70 × 34**, border-box, at x 976 (SAVE) and x 1052 (DELETE) |
| Face | `linear-gradient(#23282B,#15181A)`, 1 px `#050708`, `0 1px 0 rgba(255,255,255,.07) inset, 0 1px 2px rgba(0,0,0,.55)` |
| Legend face | Barlow Condensed 600, **10 px**, 12 px line-height, .12em tracking, .12em text-indent |
| Lit | `#F1EFEA` + **warm** `text-shadow: 0 0 7px rgba(255,229,188,.5), 0 0 2px rgba(255,246,232,.42)` |
| Unlit | `#757D82`, **no shadow at all** — matte, not a dimmer ink. **3.55:1** against the lightest part of the face (`#23282B`, top of the gradient), above the 3:1 state floor. 4.26:1 at the bottom; quote the worst case. |

The bloom is deliberately **warm** against the cool-neutral face: a warm bloom is what reads as
*backlit* rather than as brighter ink, which is the distinction the brand rule is drawing. It is not
the accent — the accent red stays with the engine lamps and appears nowhere on these buttons.

**The legends are backlit, and light individually.** The face is dark enough that a bright legend
reads as illuminated, so the indication is the legend itself rather than a lamp beside it — the
pale-face form is not used on this casting, and the two forms are never mixed. Lit is a **neutral
bright**: the engine identity colours (orange `#E5A021`, yellow `#EAD681`) stay reserved for I / II /
I+II and appear nowhere in the header.

**The button faces changed from pale to dark in this pass.** They were previously a light gradient
(`#DBE0E3 → #AAB1B6`) with dark ink, which is the face that requires the lamp-beside-legend form.
Backlit legends need a dark face, so the face moved with the treatment.

**There is no disabled face.** Where a function does not apply, its legend is simply dark:

| Panel state | SAVE | STORE | DELETE | CANCEL |
|---|---|---|---|---|
| Factory Program, unmodified | dark | dark | dark | dark |
| Factory Program, modified | **lit** | dark | dark | dark |
| User Program, unmodified | dark | dark | **lit** | dark |
| User Program, modified | **lit** | dark | **lit** | dark |
| Naming a Program | dark | **lit** | dark | **lit** |

Rules the appearance table does not imply, and which the build must not decide for itself:

| Rule | Behaviour |
|---|---|
| Escape, or CANCEL, out of naming | Leaves the Program **still modified** — nothing was stored, so the edited flag survives and SAVE lights again the moment naming exits. Do not clear it. |
| Bank tag during naming | Reads `NAME`, **not** `USER` — the Program is not in the user bank until STORE commits it. |
| STORE with an empty name | Commits as `NEW PROGRAM` rather than refusing; there is no error state on this panel. |
| Enter / Escape while not naming | Ignored entirely — these keys are only live in the naming state. |
| What sets the edited flag | Any knob move, any knob reset, any engine or page change. Cleared only by STORE and by loading a Program. |
| DELETE on a Factory Program | Dark and inert — Factory Programs cannot be deleted, and this is why the row exists rather than being folded into "unmodified". |

Both legends dark reads as "nothing to do here", never as a blank button — both stay printed and
readable. The old treatment (a dimmer fill, `opacity .55` and `#8B9297` ink on DELETE) is retired; it
measured well under the state floor and it was a typographic change standing in for a lamp.

SAVE's lamp and the LCD's trailing ` *` dirty marker **read the same flag**, so they cannot disagree.
The prototype now tracks that flag (`dirty`, set by any knob, reset, or engine change; cleared on
store and on load). **Open item:** the ` *` marker itself is not yet drawn in the LCD — it is
budgeted for (the 31-character cap reserves its two columns) but needs adding alongside this.

Stacked rather than side by side because it costs no width: each button is already sized by its
longest single word, and `DELETE` and `CANCEL` are both six characters. Side by side would roughly
double both buttons and the width would have to come out of the LCD.

**The legends stay runtime text — never baked into the button bitmap.** Each of the four can light
independently, so a baked legend would freeze one state's lighting into the face. What ships as a
bitmap is the face only; the build draws SAVE / STORE / DELETE / CANCEL live and applies the lit or
matte treatment per the table above.

Sprites: `program-save-lit`, `program-store-lit`, `program-save-dark`, `program-delete-lit`,
`program-cancel-lit`, `program-delete-dark` in `Chorus-60 Control Sprites.dc.html`, all at 70 × 34
native.

Reference renders in `reference/buttons/`, the **button pair together** at 3× — the two buttons move
as one unit, so the pair is diffable in a way separate crops are not. One per row of the table:

| File | Row |
|---|---|
| `01-factory-nothing-to-do.png` | Factory Program, unmodified — all four dark |
| `02-factory-edited-save-lit.png` | Factory Program, modified — SAVE lit |
| `03-user-unmodified-delete-lit.png` | User Program, unmodified — DELETE lit |
| `04-user-edited-save-delete-lit.png` | User Program, modified — SAVE and DELETE lit |
| `05-naming-store-cancel-lit.png` | Naming a Program — STORE and CANCEL lit |
