# CHORUS-60 CH-60 — Build Handoff

Companion to `CHORUS60-GUI-SPEC.md`. That document is the design spec (palette, type, geometry,
scales, states). **This document is the asset contract**: what ships as a bitmap, what the build
draws at runtime, and where every file lives.

Panel canvas: **1280 × 775 px** at 1× (1282 × 777 including the 1 px outer border — the exported
plate includes that border). Everything scales as a unit; @2x assets exist for every bitmap because
the GUI scales up as an accessibility feature.

Coordinates in both documents are **inside-border panel coordinates**: (0, 0) is the first pixel of
panel material, one pixel in from the exported bitmap's top-left.

---

## 1. Baked vs. runtime — read this first

**The plate is no longer glyph-free.** The previous plate carried no text at all; every string was
drawn in code over blank material. That changed with this revision: the printed scales are silkscreen
and now live in the bitmap, and the static labels went with them. Drawing them again in code will
double-print.

The rule that decides each case: **a string is baked if it never changes — neither its characters nor
its colour — for the life of the session.** The OFF-state fade doesn't count as a change, because it
is a multiply over the whole panel (plate included) rather than a per-element restyle.

### 1.1 Baked into `chorus60-background-plate` — DO NOT redraw

| Element | Notes |
|---|---|
| Panel material, outer border, top/bottom bands | Header band, body field, footer band gradients |
| Blue CHORUS stripe **including the word `CHORUS`** | 220 × 46 at (22, 96) |
| Blue bottom bar | 220 × 24 at (22, 701) |
| Vertical divider rule | 1 × 629 at (263, 96) |
| Group box outlines + their inner heading rules | MOD ENGINE, CHARACTER, OUTPUT |
| Wordmark `CHORUS-60` | Librestile Extended Bold 34 px |
| `BBD CHORUS PROCESSOR` | Barlow Condensed 600, 11 px |
| `MODEL CH-60 · STEREO` | Barlow Condensed 600, 11 px |
| Captions `PROGRAM`, `IN`, `OUT` | 9 px |
| Section heading `DELAY MODULATION` | `#E6EBEE`, static |
| Group headings `CHARACTER`, `OUTPUT` | `#A5ADB2`, static |
| **All tick rings** — every knob, all 5 ticks each | 2 px wide, rotated to point at the knob centre |
| **All printed scale numerals** — every knob | Share Tech Mono 12 px, `#A5ADB2` |
| **All unit markings** (`Hz`, `%`, `ms`, `dB`) | Barlow Condensed 600, 11 px, `#8A9196` |
| **Global control labels** `DRIFT`, `SATURATION`, `NOISE`, `MIX`, `OUTPUT TRIM` | Static strings |
| **Switch position prints** `STEREO`, `MONO` | These are values, not the control name — static |
| Empty wells: scope, PROGRAM LCD (bank cell + name field, with divider), IN and OUT meters | Inset background, border, inner shadow only — **no contents** |

### 1.2 Drawn at runtime — NOT in the bitmap

| Element | Why it can't be baked |
|---|---|
| Scope trace, grid, playhead, BBD noise floor | Live signal |
| Scope annotations `DLY MOD`, `+ MAX`, `- MAX` | Drawn onto the scope canvas each frame with its contents |
| Scope status `ENGINE I` / `ENGINE I + II` / `ENGINE BYPASS` | Changes with engine state |
| Scope status `250 ms / DIV` | Sits in the same status row; drawn with it |
| PROGRAM LCD: bank tag `FACT` / `USER`, program name, parameter readout, name-entry caret | Dynamic |
| IN / OUT numerals | Metering |
| MOD ENGINE box heading `MOD ENGINE I` / `II` / `I+II` | Page-dependent |
| MOD ENGINE status note `ENGINE I ENGAGED` / `BOTH ENGAGED · MONO BBD PAIR` / `BYPASS` | Page- and state-dependent |
| **Mod Engine control labels** `RATE I`, `DEPTH I`, `DELAY CENTER I`, `DECORRELATION I`, `IMAGE I` (and the `II` / `I+II` variants) | Page-dependent — the suffix changes |
| Engine button letters `I`, `II`, `OFF` | Colour changes with engagement (`#E6EBEE` engaged / `#A5ADB2` not) |
| Knob sprites | Filmstrip frame per value |
| Engine buttons, indicator lamps, Image switch | Sprites, see §3 |
| SAVE / STORE / DELETE / CANCEL buttons | Labels and enabled state change |
| Footer `BBD 1024 STAGE · ENGAGED · v1.0` | Contains live engine state |

**Both directions matter.** Redrawing a baked string double-prints it; baking a runtime string freezes
it. If a string is not in the table above, it is baked.

---

## 2. Background plate

| File | Size | Notes |
|---|---|---|
| `assets/chorus60-background-plate@1x.png` | 1282 × 777 | Includes the 1 px outer border |
| `assets/chorus60-background-plate@2x.png` | 2564 × 1554 | |

Source of truth: `Chorus-60 Background Plate.dc.html`. Re-export from that file if the layout moves —
it is the same markup as the panel with the runtime strings hidden, so the two cannot drift.

## 3. Control sprites

All at `assets/controls/`, `@1x` and `@2x` of each.

| File | @1x size | Placement |
|---|---|---|
| `button-ii` | 132 × 132 | (26, 183) |
| `button-i` | 132 × 132 | (26, 356) |
| `button-off` | 132 × 132 | (26, 528) |
| `lamp-on` | 96 × 96 | Drawn **centred** on the lamp position — LED centre is at (48, 48) in the sprite; the glow is baked into the transparent margin |
| `lamp-off` | 96 × 96 | Same registration |
| `switch-track` | 34 × 68 | (1147.5, 343) — empty track |
| `switch-thumb` | 26 × 26 | Drawn over the track at (1151.5, 347) for STEREO, (1151.5, 381) for MONO — **34 px of travel**, animate it |
| `switch-stereo` | 34 × 68 | Composed both-parts image, kept for reference/static use |
| `switch-mono` | 34 × 68 | Composed, as above |

Lamp centres: **(182.5, 250)** for II, **(182.5, 422.5)** for I — i.e. the `lamp-*` sprite's top-left
goes at (134.5, 202) and (134.5, 374.5). The OFF button's lamp position is empty on the hardware and
stays empty here.

The switch ships **both ways**: `switch-track` + `switch-thumb` so the thumb can actually travel its
34 px on the 260 ms spring the panel uses, and the two composed images in case a static swap is ever
easier. Use the two-part version.

**Button faces are state-independent — there is no lit/unlit pair.** On the JN-80 the buttons are
plain moulded plastic and never illuminate; the *lamp beside them* carries the state, which is why
lamp-on / lamp-off ship as a pair and the buttons do not. If the build wants a pressed state, it is a
**+3 px y translate for 110 ms**, not a sprite.

The buttons' seating shadow is **not** baked into the sprites (it would fight the plate's material).
Draw it under each button: `0 7px 13px -7px rgba(0,0,0,.95)`.

## 4. Knob filmstrips — **re-rendered: new cap, new frame boxes, @2x shipped**

The layout enlarged both knob sizes. These are re-rendered at the new diameters, not scaled sprites.

| File | Cap Ø | Frame box | Frames | Sheet | Layout |
|---|---|---|---|---|---|
| `assets/knob_mod_84px_128f.png` | **84** | **112** | 128 | 112 × 14336 | Vertical strip, frame *n* at y = −112 n |
| `assets/knob_global_68px_128f.png` | **68** | **92** | 128 | 92 × 11776 | Vertical strip, frame *n* at y = −92 n |
| `assets/knob_mod_168px_128f@2x.png` | **168** | **224** | 128 | 1792 × 3584 | **8 × 16 grid**, row-major |
| `assets/knob_global_136px_128f@2x.png` | **136** | **184** | 128 | 1472 × 2944 | **8 × 16 grid**, row-major |

⚠ **The frame box is larger than the cap.** Cap-to-box ratio is 0.75 (global is 0.739 — 68 / 0.75 is
not an integer, and 92 was chosen so the @2x sheet is an exact doubling). The margin is **not
padding**: it is where the cast shadow fades to zero. Do not re-crop the sheets, and **position each
knob from the cap centre, not from the frame box** — treating the box as the knob makes every control
render ~33 % oversized.

Earlier revisions shipped cap-fills-frame sheets (Ø84 in an 84 box). Those clipped the cast shadow
square at the frame edge — the generator's own guard flags it, and the retired Ø128 masters failed it
at border alpha 88 top / 95 bottom / 38 sides. All four sheets above pass at ≤ 2.

The @2x sheets are grids, not strips: a 224 × 28672 vertical strip exceeds the maximum texture and
canvas height on most targets. Frame *n* sits at column *n* mod 8, row ⌊*n* / 8⌋.

Frame 0 = −135°, frame 127 = +135°, linear in between.

**Source: a parametric generator, not a raster master.** The knob is drawn programmatically from a
scale factor S — cap radius, rim stroke (2 × S), shadow blur (11 × S), shadow offset (8 × S), pointer
width/length and pointer glow all derive from it. Any diameter is reachable by re-running it; the
retired Ø128 files were outputs, never the source. All four sheets above were produced from that
generator at Chorus-60 diameters, natively at each size — nothing is upscaled.

**The cap look changed this revision, deliberately.** Chorus-60 moved onto Gatecrasher's current
render (the knob is shared — the retired Ø128 masters were byte-identical across both projects), then
diverged on two points, both recorded in `tools/render-knob-filmstrips-chorus60.mjs`:

- **Darker cap, lit from the centre.** Gatecrasher lights the cap off-axis from the upper left,
  which reads grey and flat against Chorus-60's dark fascia. Chorus-60 uses a **centred radial
  falloff** instead — grey core, black rim, as on the original CH-60 knob. Gradient centred on the
  cap at radius 0.62 × D with stops `#5a626a / #2d333a / #0e1114 / #030405`; top highlight .13 → .06
  and bottom occlusion .28 → .18, since the vignette now does that work; knurl shadow band .18 → .22.
- **Both pointers reach the cap centre.** The inherited geometry had the two knobs disagreeing: the
  global pointer was thick and overshot (`pointerW` 0.075, tip at 0.625 D) while the mod pointer
  stopped short at 0.468 D. Both now land at **0.520 D** — global `pointerW` 0.056 / `pointerTop`
  0.115 / `pointerLen` 0.405, mod `pointerW` 0.048 / `pointerTop` 0.081 / `pointerLen` 0.439.
- **The knurl rotates with the frame.** Gatecrasher draws the skirt at fixed angles, so the flutes
  sit still while the pointer moves — visibly wrong once you turn a knob. Here the serration offset
  advances by the frame angle.

Both @1x and @2x were re-rendered. This is a visible change to the panel, not a resolution-only swap
— the renders in `reference/` predate it.

Retina is no longer an open dependency. The generator ships with the bundle
(`tools/render-knob-filmstrips-chorus60.mjs`, `npm i canvas` then `node …`), so any future size is a
re-run rather than an upscale. It carries the same shadow guard: all four sheets pass at ≤ 2.

## 5. Product icon — **exists, unchanged**

Already delivered and not touched by this revision. Do not regenerate.

| File | Size |
|---|---|
| `assets/icon/chorus60-icon-1024.png` | 1024 |
| `assets/icon/chorus60-icon-512.png` | 512 |
| `assets/icon/chorus60-icon-256.png` | 256 |
| `assets/icon/chorus60-icon-128.png` | 128 |
| `assets/icon/chorus60-icon-64.png` | 64 |
| `assets/icon/chorus60-icon-32.png` | 32 |
| `assets/icon/chorus60-icon-16.png` | 16 |
| `assets/icon/chorus60-icon-light-512.png` | 512, light-plate variant |

Design: the doubled `60` numeral from the wordmark — the plugin's own signature element, red ghost
offset behind white — over the panel's material, with the blue CHORUS stripe beneath it. Reviewed at
32 px: the stripe drops below 48 px and the inner hairline below 128 px. Source:
`Chorus-60 Icon.dc.html`.

## 6. Reference renders — for review only, not build inputs

| File | State |
|---|---|
| `assets/chorus60-page-i@2x.png` | Engine I |
| `assets/chorus60-page-ii@2x.png` | Engine II |
| `assets/chorus60-page-i-plus-ii@2x.png` | Both engaged |
| `assets/chorus60-page-off@2x.png` | Bypass — **re-exported**: 0.50 multiply applied to the three control groups, pointers held at the Engine-I values, no caption. The three group regions measure 0.498 × the corresponding regions of `chorus60-page-i@2x.png`, so it is a usable acceptance target |

These are full composites — plate plus every runtime element. Use them to check the assembled result,
never as a source to slice.

One note on the bypass render: the reference implementation approximates the OFF multiply with CSS
`opacity`, which the export pipeline does not carry into a bitmap, so the multiply is composited into
that PNG directly. The PNG — not the CSS — is the acceptance target, and it is a true multiply as
§9 specifies.

## 7. Never export

The scope trace, the PROGRAM LCD text (program name, bank tag, parameter readout), and the IN / OUT
numerals are dynamic. Their **wells** are in the plate; their **contents** are not.

## 8. Everything else the build needs

In `CHORUS60-GUI-SPEC.md`:

- §2 palette — hex values and measured contrast ratios per role. Runtime-drawn text must match the
  baked text exactly, so these are not suggestions.
- §3 typography — face, size and tracking per role.
- §4 layout — every region's x / y / w / h.
- §5 LCD — window and name-field geometry, 9.6 px per character, 36-character budget.
- §7 printed scales — per-knob marks, sweep fractions and tick angles, and the numeral placement rule.
- §7.1 Rate's power-law anchors.
- §7.2 the IMAGE rename (`mono1`/`mono2`/`monoB` → `image1`/`image2`/`imageB`).
- §8 knob positions.
- §9 the OFF state — 0.50 as a multiply, pointers held, no caption.
- §10 the paged MOD ENGINE contract.
- §11 parameters, ranges, defaults, skews.
- §12 the unchanged list.

## 9. Source files

| File | Role |
|---|---|
| `Chorus-60.dc.html` | Reference implementation — the panel, fully interactive. The behavioural spec |
| `Chorus-60 Background Plate.dc.html` | Plate source. Re-export from here |
| `Chorus-60 Control Sprites.dc.html` | Sprite source — buttons, lamps, switch |
| `Chorus-60 Icon.dc.html` | Icon source and size ladder |
| `assets/LibrestileExtBold.ttf` | Wordmark face |
