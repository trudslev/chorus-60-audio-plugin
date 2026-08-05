# CHORUS-60 CH-60 — GUI Implementation Spec

Panel: **1400 × 632 px** at 1× (fixed aspect, 2.22:1 — the same ratio as Gatecrasher's 960 × 434,
per BRAND.md's fixed-aspect-canvas rule). Reference renders in `assets/`.

**Background plate:** `assets/chorus60-background-plate@2x.png` (2804 × 1266, draw at 1400 × 632)
is the full static fascia with **no controls and no text over top** — panel material and frame, header
chrome with empty PROGRAM / IN / OUT wells, blue stripes and the blank `CHORUS` strip, section divider,
empty scope well, and the empty group boxes with their heading rules. Every glyph on the panel is drawn
by the host, not baked in: the wordmark and model lines, all captions, section headings, knob labels,
button legends (I / II / OFF) and the footer status line — plus the three chorus buttons, all six LEDs,
the nine knob filmstrips and value readouts, the scope trace and annotations, PROGRAM text +
SAVE/DELETE, and the IN/OUT numbers. Text positions are unchanged from the plate's geometry (label
boxes are reserved at their measured heights), so §§2–8 coordinates apply as written. Blit the plate
once as the editor background. Re-render from `Chorus-60 Background Plate.dc.html` if the fascia changes.
**Product icon:** `assets/icon/` — dark plate at 1024 / 512 / 256 / 128 / 64 / 32 / 16 px plus
`chorus60-icon-light-512.png` for light hosts and print. Mark is the Librestile `60` in `#EEF2F4` with a
`#FF2B1C` ghost copy offset down-right (the chorus double), over the panel chassis gradient and a
`#2F6DB8` → `#1F5798` stripe. Proportions: glyph cap size 41.4% of tile, ghost offset 5.1% x / 3.1% y,
corner radius 20.3%, stripe inset 20.3% and 1.6% tall at 15.2% up from the bottom. The stripe is
dropped at 48px and below, the inner hairline frame below 128px, and the ghost stays fully opaque
below 128px (it is 90% at larger sizes). Re-render from `Chorus-60 Icon.dc.html`.
All coordinates below are panel-local, origin = top-left of the 1400 × 632 panel.
Suite sibling: Gatecrasher GR-85 — where a component exists in both plugins the Gatecrasher spec
governs its construction and this document only states what differs.

---

## 1. Palette

| Role | Value |
|---|---|
| Chassis (vertical gradient, top→bottom) | `#141618` → `#0E1012` @35% → `#0A0C0D` |
| Header band | `#191C1E` → `#101214`, bottom border `#000000` + 1px `rgba(255,255,255,.05)` highlight below |
| Body field | `#0E1012` → `#0A0C0D` |
| Group panel | `#131517` → `#0C0E10`, 1px `#000` border, 1px `rgba(255,255,255,.05)` inner top highlight |
| Footer band | `#131517` → `#0C0E10` |
| Engraved heading text | `#E6EBEE` |
| Control label text | `#8A9196` |
| Value text | `#C6CED3` |
| Caption / tertiary | `#7B8287` |
| Inactive label | `#5F666B` |
| Footer text | `#5A6165` |
| LED window bg | `#07090A`, border `#363C41`, inset shadow `rgba(0,0,0,.9)` |
| LED window text | `#DFE6EA`, glow `rgba(200,220,230,.25)` |
| Section divider | 1px `rgba(0,0,0,.9)` (fades at ends) + 1px `rgba(255,255,255,.06)` highlight to its right |
| **Section stripe (structural)** | `#2F6DB8` → `#1F5798`, stripe text `#EEF2F4` |
| Engine button II | `#E5A021` → `#C07908` (160°) |
| Engine button I | `#EAD681` → `#D0B857` (160°) |
| OFF button | `#EAECEC` → `#C9CDCF` (160°) |
| **Chorus accent (ONLY colour beyond the stripes)** | `#FF2B1C` |
| LED unlit | `#3A1512` |

**Rule: red is reserved for engine state.** The two engine LEDs, the DELAY MODULATION lamp and the
scope trace are the only red elements on the panel. The blue stripes are structural framing from the
hardware, not an accent — they appear twice, above and below the button column, and nowhere else.

## 2. Typography

- Labels / headings: **Barlow Condensed** (600 for labels, 700 for group/lamp text) — same as Gatecrasher.
- Numeric readouts, LED windows, scope annotations: **Share Tech Mono** — same as Gatecrasher.
- Wordmark: **Librestile Extended Bold** (`assets/LibrestileExtBold.ttf`, free for commercial use;
  ship in BinaryData) — see §8.
- Sizes: wordmark 34px · model line 11px / .24em · section headings 10px / .28em · scope lamp label
  11px / .28em · control labels 10px / .18em · values 11px · LED program 13px / .10em · tag cell
  9px / .12em · captions 9px / .24em.

## 3. Layout

| Region | x | y | w | h |
|---|---|---|---|---|
| Header band | 1 | 1 | 1400 | 75 |
| Body | 1 | 76 | 1400 | 526 |
| Footer band | 1 | 602 | 1400 | 29 |
| Button column | 25 | 94 | 232 | 488 |
| Section divider | 279 | 94 | 1 | 488 |
| Control column | 302 | 94 | 1075 | 488 |

Body padding: 18 top / 24 sides / 20 bottom. Control column stacks with 16px gaps:
scope caption row (h 21) → scope (h 124) → engine row (h 159) → lower row (h 143).

## 4. Button column — the hardware chorus section

Straight from `assets/jn80-chorus-reference.jpeg`, expanded to fill the column.

| Element | x | y | w | h |
|---|---|---|---|---|
| Top stripe (contains `CHORUS`) | 25 | 94 | 232 | 24 |
| Button II | 31 | 144 | 116 | 116 |
| LED II | 167 | 195 | 15 | 15 |
| Button I | 31 | 286 | 116 | 116 |
| LED I | 167 | 337 | 15 | 15 |
| Button OFF | 31 | 428 | 116 | 116 |
| Bottom stripe | 25 | 570 | 232 | 12 |

- Stripe caption `CHORUS`: Barlow Condensed 700, 13px, .40em tracking, `#EEF2F4`, centred with
  `text-indent:.40em` so the trailing letter-space doesn't push it optically left.
- Buttons: 5px radius, gradient per §1 at 160°, 1px top inner highlight
  (`rgba(255,255,255,.40)` II / `.50` I / `.85` OFF), inner bottom shade
  `0 -7px 12px -4px rgba(0,0,0,.35–.50)`, drop shadow `0 7px 13px -7px rgba(0,0,0,.95)`.
  On press: translate down 3px for 110ms, then release. State is latching, not momentary.
- Roman labels sit 27px right of each LED: Barlow Condensed 700, 22px, .06em; `#E6EBEE` when that
  engine is engaged, `#8A9196` when not. `OFF` is 18px / .14em, `#E6EBEE` when nothing is
  engaged and `#7B8287` when an engine is on.
- LEDs (Ø15): unlit `#3A1512` with inset `rgba(255,255,255,.14)`; lit radial
  `#FF2B1C` → `#B0140C` @70% → `#6D0B06` with glow `0 0 12px 3px rgba(255,43,28,.55)` and
  `0 0 30px 8px rgba(255,43,28,.22)`. Same lamp component as Gatecrasher's GATE OPEN lamp.
- Logic: `engine1` and `engine2` are independent latches. OFF clears both. Both engaged is the
  legitimate "I+II" hardware trick and must be reachable.

## 5. Delay-modulation scope — the centrepiece

Caption row (y 94, h 21): `DELAY MODULATION` at x 302 (Barlow Condensed 700, 11px, .28em;
`#E6EBEE` engaged / `#7B8287` bypassed). There is **no lamp in this row** — per BRAND.md a panel
carries exactly one live-state indicator, and on this plugin that indicator is the engine LED pair in
the button column (§4). Right-aligned, three Share Tech Mono 11px `#7B8287` readouts spaced 26px:
`ENGINE I` / `ENGINE II` / `ENGINE I + II` / `ENGINE BYPASS`, `DEPTH nn%` (sum of the
engaged engines' depth), and the division label `250 ms / DIV`.

Scope rect: **x 302, y 115, w 1077, h 124**. 1px `#0A0C0D` border, bg vertical `#06080A` → `#0B0F11`,
inset shadow `0 2px 7px rgba(0,0,0,.85)`. Same construction as Gatecrasher's envelope scope.

- **Window**: 2.0 s of history, scrolling right-to-left at 60fps. 8 vertical divisions scroll with
  the signal (hence `250 ms / DIV`); 6 horizontals are static. Grid `rgba(150,180,190,.10)`,
  centre line `rgba(150,180,190,.22)`.
- **Trace**: 3px `#FF2B1C`, glow pass underneath at 7px `rgba(255,43,28,.45)` with a 20px shadow
  in `rgba(255,43,28,.80)`, then the core pass with a 10px shadow. Mitre joins. A 4px filled dot
  `rgba(255,43,28,.90)` sits at the right edge on the current sample.
  Unlike Gatecrasher's gate envelope this waveform is genuinely curved — do **not** flatten or
  quantise it, and do not smooth it either: it is the modulator, sampled.
- **Geometry**: amplitude 0.34 × h; vertical centre offset by `0.10 × h × ((delayCenter − 8) / 6)`
  so raising Delay Center visibly moves the whole trace down the well.
- **Signal**: sum of the engaged engines. Engine I `depth1/100 · sin(2π·rate1·t)`; engine II the
  same with a +1.1 rad phase offset; plus drift `drift/100 · 0.14 · sin(2π·0.11·t + 2.3)`; plus
  clock noise `noise/100 · 0.045 · mean(sin(811.7t), sin(1531.3t))`. In the plugin, feed the scope
  the **real modulator output** — this formula is the mockup's stand-in and defines the look, not
  the DSP.
  With both engines off the trace does not go dead flat: it settles onto the drift/noise floor.
- **Dry input underlay**: behind the trace, 1px vertical strokes at `rgba(178,190,197,.22)` every
  5px, amplitude `0.20 × h × env × (0.35 + 0.65·noise)` about the trace's centre line — the input
  signal the modulation is acting on, so the display reads as a diagnostic instrument. Grey, never
  red, and always behind the trace.
- Annotations in Share Tech Mono 9px `rgba(160,178,186,.55)`: `DLY MOD` top-left,
  `+ MAX` / `- MAX` right-aligned top and bottom.

## 6. Program section (header, right side)

Contract is **identical to Gatecrasher §6** — same window construction, same button states, same
name-entry flow. Only the coordinates and the dark surround differ.

| Element | x | y | w | h |
|---|---|---|---|---|
| `PROGRAM` caption | 832 | 17 | — | 11 |
| Program window (outer) | 832 | 34 | 307 | 27 |
| &nbsp;&nbsp;FACT / USER tag cell | 833 | 35 | 43 | 25 |
| &nbsp;&nbsp;Name cell | 876 | 35 | 262 | 25 |
| SAVE / STORE button | 1145 | 34 | 41 | 27 |
| DELETE / CANCEL button | 1192 | 34 | 51 | 27 |
| IN window | 1261 | 34 | 54 | 27 |
| OUT window | 1323 | 34 | 54 | 27 |

- Tag cell: Share Tech Mono 9px / .12em — `FACT` in `#6F797F`, `USER` in `#CFD7DC`. Read-only.
- Name cell: Share Tech Mono 13px / .10em `#DFE6EA`, centred, showing `NN NAME` (two-digit
  program number, space, name). Left-aligns in name-entry mode.
- SAVE / DELETE: stamped-steel utility buttons, Gatecrasher's exact treatment — enabled
  `#DBE0E3` → `#AAB1B6` / border `#6D7478` / label `#22272B`; disabled `#C2C8CC` → `#A8AFB3`
  / border `#8D9498` / label `#8B9297` at .55 opacity. Labels Barlow Condensed 600, 9px, .12em
  tracking + matching text-indent. They are the only light-steel elements on this panel and that is
  intentional — the utility surface is shared across the suite.
- IN / OUT: LED windows, Share Tech Mono 13px, signed dBFS to one decimal. Numeric only — no bar
  graph on this plugin. Update at ~6 Hz. `OUT = IN + trim` (+~1.1 dB of chorus makeup when engaged).
- Behaviour: factory program → tag `FACT`, SAVE enabled, DELETE disabled. User program → tag
  `USER`, both enabled, DELETE reverts to the factory program. SAVE → name-entry: caption becomes
  `NAME PROGRAM`, tag switches to `USER`, name cell clears and left-aligns with a blinking block
  caret (`█`, 1s steps, 50% duty), buttons relabel `STORE` / `CANCEL`, typing is uppercased and
  capped at 22 chars, Enter stores, Esc cancels, empty name falls back to `NEW PROGRAM`.
  Reference renders: `assets/header-factory-program@3x.png`, `assets/header-user-program@3x.png`,
  `assets/header-name-entry@3x.png`.

## 7. Knobs

Rotation range for every knob: **−135° → +135°** (270° sweep), pointer at 12 o'clock = centre —
same as Gatecrasher, same two filmstrips.

| Control | cx | cy | Ø | Filmstrip |
|---|---|---|---|---|
| RATE I | 454 | 328 | 58 | large |
| DEPTH I | 680 | 328 | 58 | large |
| RATE II | 999 | 328 | 58 | large |
| DEPTH II | 1226 | 328 | 58 | large |
| DELAY CENTER | 385 | 495 | 42 | small |
| DECORRELATION | 521 | 495 | 42 | small |
| DRIFT | 703 | 495 | 42 | small |
| SATURATION | 839 | 495 | 42 | small |
| NOISE | 976 | 495 | 42 | small |
| MIX | 1158 | 495 | 42 | small |
| OUTPUT TRIM | 1294 | 495 | 42 | small |

Knob size communicates importance (BRAND.md): the four modulation knobs — the plugin's character
controls — are the large strip at Ø58; every shared/tone-shaping control is the small strip at Ø42.

Filmstrips: `assets/knob_large_128px_128f.png` and `assets/knob_small_128px_128f.png`,
128 × 16384, 128 frames, frame 0 = −135°, frame 127 = +135°, linear. Frames are square with
transparent margin and a baked cast shadow — draw into the knob's full bounding box, not the circle.

```cpp
const int frame = juce::jlimit(0, 127, (int) std::round(sliderPos * 127.0f));
g.drawImage(strip,
            bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
            0, frame * 128, 128, 128);
```

Use `Graphics::setImageResamplingQuality (highResamplingQuality)`.

Tick ring: drawn in code, **not** part of the filmstrip. 1px radial ticks in `rgba(120,132,140,.55)`
from r+3 to r+9, every 15° on the large knobs / 20° on the small ones, spanning the 270° sweep only —
no ticks across the bottom 90°.

Label stack beneath each knob: name (Barlow Condensed 600, 10px, .18em, `#8A9196`) then value
(Share Tech Mono 11px, `#C6CED3`), 9px gaps.

Interaction: `Slider::RotaryVerticalDrag`, full range over 180px of travel, Shift = 0.25× fine,
double-click resets to default (`setDoubleClickReturnValue`).

Group panels (1px `#000`, gradient per §1, 9/14/14 padding, title row over a hairline rule):

| Group | x | y | w | h | Title row |
|---|---|---|---|---|---|
| MOD ENGINE I | 302 | 255 | 530 | 159 | Ø8 LED (engine I state) + title |
| MOD ENGINE II | 848 | 255 | 530 | 159 | Ø8 LED (engine II state) + title |
| BBD LINE | 302 | 430 | 302 | 143 | title only |
| CHARACTER | 620 | 430 | 438 | 143 | title only |
| OUTPUT | 1075 | 430 | 302 | 143 | title only |

Knobs are distributed `space-evenly` inside each group.

## 8. Wordmark

Nameplate metaphor (BRAND.md requires a distinct one per plugin): **silkscreened synth-panel legend** —
ink laid flat on the fascia by a screen printer, the way a Juno's or JN-80's section names are. Not
TapeRot's Dymo tape, not Gatecrasher's spray stencil, and explicitly **not distressed**: the JN-80-era
panel it comes from is clean and precise.

Set `CHORUS-60` live in Librestile Extended Bold, 34px at 1×, .02em tracking, `#E6EBEE`, at x 25,
y 30 (block 308 × 31), with `0 1px 0 rgba(0,0,0,.9)` engraving shadow plus a 1px
`rgba(230,235,238,.5)` ink bloom — the faint spread of screen-printed ink, and the only texture the
wordmark gets. No spray, no speckle, no per-letter rotation, no halo. Ship the TTF in BinaryData and
draw it as text; baking to PNG is fine, but the treatment must stay flat.

Model line, right of the wordmark at x 351: `BBD CHORUS PROCESSOR` (`#8A9196`) over
`MODEL CH-60 · STEREO` (`#5F666B`), Barlow Condensed 600, 11px, .24em, 4px apart.

## 9. Parameters

| ID | Name | Range | Default | Skew / notes |
|---|---|---|---|---|
| `engine1` | Engine I | off / on | on | latch — yellow button + LED I |
| `engine2` | Engine II | off / on | off | latch — orange button + LED II |
| `rate1` | Rate I | 0.05 → 8 Hz | 0.45 | log (skew ≈ .35) |
| `depth1` | Depth I | 0 → 100 % | 38 | linear |
| `rate2` | Rate II | 0.05 → 8 Hz | 2.90 | log (skew ≈ .35) |
| `depth2` | Depth II | 0 → 100 % | 64 | linear |
| `delayCenter` | Delay Center | 2 → 14 ms | 5.6 | linear — BBD tap centre; offsets the scope trace |
| `decorrelation` | Decorrelation | 0 → 100 % | 52 | linear — L/R modulator phase offset, 0 % mono-linked → 100 % 180° apart |
| `drift` | Drift | 0 → 100 % | 22 | linear — slow clock wander, visible in the trace |
| `saturation` | Saturation | 0 → 100 % | 30 | linear — BBD stage drive |
| `noise` | Noise | 0 → 100 % | 14 | linear — clock noise, visible in the trace |
| `mix` | Mix | 0 → 100 % | 50 | linear |
| `trim` | Output Trim | −12 → +12 dB | 0 | linear, signed display |

Both engines running simultaneously is the classic Juno "I+II" state — allow it; the depth readout
sums and the scope shows the composite.

Factory programs: Wide Ensemble, Juno I, Juno II, Juno I+II, Slow Swell, Vibrato, Dimension,
Shimmer Pad, Clock Noise, Warped Tape, Deep Detune, String Machine, Bright Doubler, Mono Verify,
Cold Chorus, Dark Ensemble.
Default program on load: `07 WIDE ENSEMBLE` (factory).

## 10. Suggested structure

```
Source/
  PluginProcessor.{h,cpp}       // APVTS, BBD line, dual mod engines, drift/noise, meters
  PluginEditor.{h,cpp}          // 1180x714 root, aspect-locked scaling
  ui/PanelLookAndFeel.{h,cpp}   // filmstrip knobs, tick rings, fonts, colour IDs
  ui/ChorusButtonColumn.{h,cpp} // blue stripes, three square buttons, engine LEDs
  ui/ModScope.{h,cpp}           // scrolling grid, annotations, glow+core red trace, 60 fps
  ui/ProgramDisplay.{h,cpp}     // LCD window, tag cell, SAVE/DELETE, name entry  (port from Gatecrasher)
  ui/LevelReadout.{h,cpp}       // IN / OUT LED windows                            (port from Gatecrasher)
  ui/KnobGroup.{h,cpp}          // titled panel + N labelled knobs
  dsp/BBDLine.{h,cpp}           // 1024-stage BBD, clock noise, saturation
  dsp/ModEngine.{h,cpp}         // rate/depth/phase, drift
```

Anything in `ProgramDisplay`, `LevelReadout` and the knob LookAndFeel should be lifted from the
Gatecrasher project rather than rewritten — the two plugins are meant to be recognisably the same
instrument family, with only the fascia, the accent placement and the control set differing.

## 11. What matters most

1. The scope is the product. Real modulator signal, real red glow, moves the instant a knob moves.
2. The button column must read as the hardware photo — colour, size, blue stripes, LED position.
3. Red appears nowhere except the LEDs, the lamp and the trace. Blue appears only in the two stripes.
4. Sharp corners everywhere except the three buttons, the knobs and the LEDs.
5. The panel must feel unbolted from a synth, not racked — no ears, no screws, near-black material.

## 12. BRAND.md compliance notes

Checked against `BRAND.md` (Neon Foundry shared DNA). Where this panel deviates, it is deliberate
and recorded here — do not "fix" these in implementation.

**Followed**
- Fixed-aspect canvas at the reference ratio: 1400 × 632 = 2.22:1, matching Gatecrasher's
  960 × 434 = 2.21:1.
- One live-state indicator system, one accent colour: the engine LEDs and the scope trace, both
  `#FF2B1C`, used nowhere else. The redundant lamp that previously sat in the scope caption row was
  removed for exactly this reason.
- Shared component grammar: same filmstrip knobs and tick-ring construction as Gatecrasher, knob size
  by importance, real oscilloscope readout with grid, `250 ms / DIV` division label and a subtle grey
  input underlay behind the trace.
- Hardware voice: `BBD CHORUS PROCESSOR` / `MODEL CH-60 · STEREO`, `BBD 1024 STAGE` footer
  stamp, `v1.0` corner version, real ms/Hz/%/dB values under every knob, numeric IN/OUT in dB.
- **Programs**, never "presets" — UI label, tag cell, factory bank and this document all say Program.
- Distinct nameplate metaphor (§8) and a fascia material chosen for the plugin's era rather than
  matched to a sibling.
- No umbrella brand name on the panel.

**Deliberate deviations**
1. **Two LEDs, not one.** BRAND.md asks for a single dedicated lamp. The hardware this plugin models
   has one LED per chorus engine, and which engine is running is the discrete state worth reporting.
   They are treated as one indicator *system* in one accent colour, and no third lamp exists on the
   panel.
2. **Three non-neutral button faces.** The orange II / tan I / white OFF buttons are the hardware's
   own colours and the whole point of the button column; they are switch caps, not decoration, and
   they carry no accent red.
3. **The blue section stripes.** Structural framing lifted from the hardware panel, used exactly
   twice — above and below the button column — to bound that section. They are not an accent and
   never appear elsewhere. If a future audit wants strict single-colour discipline, this is the one
   item to revisit; the design intent is that they read as printed panel graphics, like the coloured
   section bands on the real instrument.
