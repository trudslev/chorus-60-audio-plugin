# CHORUS-60 — GUI SPEC

Model **CH-60**, BBD stereo chorus. Neon Foundry casting, harmonisation round.
Authoritative for the build.

**Read `shared/HEADER-PART.md` first.** The block, the band, the LCD cell with its budget and
cap, the Program buttons and their state matrix, and the meter wells are the shared part and
are not restated except where this casting's material meets them.

**Asset format: vector / code-drawn, on an exported fascia plate, plus two control sprites.**
The plate carries the fascia gradient, the CHORUS badge and the box frames; every knob, lamp,
label, tick, numeral and the scope are drawn at runtime. The IMAGE switch is a two-state
hardware part and ships as sprites. Nothing carrying a live value is baked.

---

## 1 · Canvas

| Figure | Value |
|---|---|
| Canvas | **1340 × 812** at 100 % |
| Fascia | `linear-gradient(180deg, #141618, #0e1012 55%, #0a0c0d)`, `inset 0 0 0 1px #000` |
| Header block | 16, 16, 1308 × 104 — shared part, material `linear-gradient(180deg, #24292c, #171a1c)` |
| CHORUS badge | 24, 136, 238 × 46 — `linear-gradient(100deg, #2a4cba, #1b3593)`, and a 238 × 26 foot bar at y 740 |
| Engine column | 24, 196, 132 × 534 |
| Scope | 285, 160, 1039 × 120, header row at y 136 |
| MOD ENGINE box | 285, 296, 1039 × 240 |
| CHARACTER box | 285, 552, 567 × 214 |
| OUTPUT box | 868, 552, 456 × 214 |
| Footer | y 782 |

**Call 1 brought +58 px** (1282 → 1340), absorbed by the two lower boxes.

---

## 2 · The engine column — this casting's signature layout

Three buttons at 132 × 132 in the left column, each with its own cap colour and a lamp-plus-legend
row to its right at 148 px.

| Button | y | Cap | Legend |
|---|---|---|---|
| **II** | 34.5 | `linear-gradient(160deg, #e5a021, #c07908)` | 22 px |
| **I** | 201 | `linear-gradient(160deg, #ead681, #d0b857)` | 22 px |
| **OFF** | 367.5 | `linear-gradient(160deg, #eaecec, #c9cdcf)` | 18 px |

Column spacing follows the JN-80: four equal 41 px gaps, **distributed rather than centred.**

**OFF carries no lamp** — it is the absence of a selection, not a state with its own light —
but **its lamp slot is held open** so all three legends share one left edge. Legend ink is
`#e6ebee` when that engine is live and `#a5adb2` when it is not; the legend never changes size
or weight.

Lamps Ø15: lit `radial-gradient(circle at 40% 32%, #ff5a4a, #ff2b1c 38%, #b0140c 72%, #6d0b06)`
with `inset 0 0 6px 1px rgba(255,43,28,.5)` and an 8 px outer glow; unlit
`radial-gradient(#4a2320, #2e1513 60%, #1c0d0b)`, no glow. **Light stops at the lens edge**, and
the unlit lens stays a dark red lens rather than going grey.

### 2.1 Pages — I · II · I+II · OFF

The MOD ENGINE row **is** the page: its four dials are whichever engine the buttons select, and
selecting a page moves the pointers rather than adding or removing controls.

| Page | II lamp | I lamp | Trace rate · depth |
|---|---|---|---|
| I | unlit | **lit** | 0.11 · 0.38 |
| II | **lit** | unlit | 0.30 · 0.62 |
| I+II | **lit** | **lit** | 0.19 · 0.52 + a second sine at 0.31 × 0.35 |
| OFF | unlit | unlit | depth 0 — flat line |

**No panel text relabels itself on a page change.** Knob labels are plain control names; the
lamps say which engine is live. That is the whole mechanism, and it is why the row can be one
set of four dials rather than three.

---

## 3 · Knobs — two classes

| Class | Ø | Controls | Pivot y |
|---|---|---|---|
| Primary | **76** | RATE (x 425), DEPTH (621), DELAY CENTER (817), DECORRELATION (1013) | **416** |
| Standard | **56** | DRIFT (389), SATURATION (569), NOISE (749), MIX (1006), OUTPUT TRIM (1186) | **660** |

Cap `radial-gradient(circle at 36% 26%, #191d21, #0c0e10 48%, #010202)`, rim
`inset 0 0 0 1.5px #000`, specular a rotated ellipse at `inset: 6px` — **fixed to the panel,
not to the knob**, and it does not rotate. Pointer 3 × (r − 1), `#e6ebee`, which measures
**16.10:1** against the cap's outer field: the widest pointer separation in the suite.

Sweep 270°, angle = `−135 + 270 f`. Ticks major **2 × 9**, minor **1.5 × 5**, ink `#a5adb2`.
Numeral ring at `r + 29.5`, Share Tech Mono **11 px / 13**.

**No mixed-class row exists on this panel** — the Ø76 dials are all on y 416 and the Ø56 dials
all on y 660 — so the registration rule (`dy = (larger − smaller) / 2`) does not arise here.
Stated because a call that cannot apply should say so rather than be silent.

### 3.1 Mark lists

| Knob | Ø | Marks |
|---|---|---|
| RATE | 76 | skewed — see §3.2 |
| DEPTH · DECORRELATION | 76 | even fifths, **0 / 25 / 50 / 75 / 100** % |
| DELAY CENTER | 76 | even fifths, **2 / 5 / 8 / 11 / 14** ms |
| DRIFT · SATURATION · NOISE · MIX | 56 | **0 / 50 / 100** %, minors at .25 and .75 |
| OUTPUT TRIM | 56 | **−12 / 0 / +12** dB, minors at .25 and .75, leading plus kept |

Standard class carries three numerals and the demoted positions keep their ticks. Units print
inside the arc's bottom gap, never as a suffix on the control name. `−` is U+2212.

### 3.2 RATE

RATE's ring is skewed and its mark list is the one in catalogue §10 — **the fractions are the
contract and must not be derived by even spacing.** Its pointer position is per page (0.2869
on I, 0.5600 on II, 0.4200 on I+II), which is what makes the page switch visible on a row that
does not relabel.

---

## 4 · Scope — DELAY MODULATION

1039 × 120 at (285, 160), `linear-gradient(180deg, #06080a, #0b0f11)`, `inset 0 0 0 1px #000`.
Header row above it at y 136: title **DELAY MODULATION** (Barlow Condensed 700, 12 / 15 / .28 em,
`#e6ebee`) and `250 ms / DIV` at the right.

| Element | Spec |
|---|---|
| Grid | vertical `rgba(255,43,28,.10)` every 129.875, horizontal `rgba(255,255,255,.05)` every 30 |
| Zero line | y 59.5, `rgba(255,255,255,.10)` |
| Trace | `#ff2b1c`, 1.8 px, `drop-shadow(0 0 6px rgba(255,43,28,.5))` — 5.15:1, graphic |
| Annotations | `DLY MOD`, `+ MAX`, `− MAX` — Share Tech Mono 11 / 14 / .06 em, `#9ba3a8`, **7.52:1** |

**The annotations are drawn opaque at a single value.** They were `rgba(160,178,186,.55)` at
3.11:1 — opacity-driven hierarchy, which the brand forbids — and hierarchy is now carried by
position alone.

---

## 5 · IMAGE switch

At (1112, 382), 128 wide: a 34 × 68 sprite with both legends printed to its right, STEREO above
MONO, 10 px / 13 / .14 em `#a5adb2` — **8.61:1**.

Two sprites, one per state, `assets/chorus60/switch-stereo@2x.png` and `switch-mono@2x.png`,
each given a **literal `src`**: a hole in an `img src` is fetched verbatim before values
resolve, so the state is selected by which image is rendered, not by a computed path.

**Both legends are printed permanently and neither moves or re-inks** — the switch's own
position is the state. This is §4B's rule applied to a sprite part rather than a drawn shoe.

Sprites are **3× as of the second bundle** — 102 × 204 from the drawn 34 × 68, redrawn rather than
traced because the delivered 2× pair predated this round's ink pass. **The `@2x` in both filenames
is now stale**; the names are the build's asset paths, so renaming them is the build's call.

---

## 6 · Palette and measured contrast

Computed in one pass from this panel's own hexes against each ground **by name**, worst case
where the ground is a gradient. Functional 7:1, flavour 4.5:1, state 3:1.

### On the header block (worst, i.e. lightest, `#24292c`)

| Ink | Role | Ratio |
|---|---|---|
| `#e6ebee` | wordmark, function descriptor | **12.24** |
| `#b6bec2` | model line, PROGRAM caption, IN / OUT captions | **7.79** |

`#b6bec2` is the reconciled hex. The body carried `#8a9196` for this role at **4.60** — a
functional string more than a stop and a half under its floor — while the six-material strip
already had `#b6bec2`. Reconciled to the strip's; see §8.

### On the section boxes (worst `#131517`)

| Ink | Role | Ratio |
|---|---|---|
| `#e6ebee` | box titles, live engine legend | **15.24** |
| `#a5adb2` | control labels, scale numerals, **printed units**, inactive engine legend | **8.04** |

**The printed units were `#8a9196` at 5.73 and are now `#a5adb2`.** A unit is part of the
printed scale — it says what the numerals mean — so it carries the scale's ink and the scale's
floor, not a caption's. That is the third instance this round of a role being classified one
step too low; the other two were model lines.

### On fascia (`#0a0c0d`) and the badge (`#1b3593`)

| Ink | Role | Ratio | Class |
|---|---|---|---|
| `#a5adb2` | IMAGE legends | **8.61** | functional |
| `#dfe4ec` | CHORUS badge | **8.32** | functional |
| `#8a9196` | footer — `CH-60 · SN 0061`, `CHORUS-60 · v1.0` | **6.13** | flavour |

### On LCD glass (`#07090a`)

| Ink | Role | Ratio |
|---|---|---|
| `#dfe6ea` | program name, bank tag, live readout, meter values, chevron | **15.81** |

### On the Program cap (`#23282c → #14181b`)

| Ink | State | Ratio |
|---|---|---|
| `#f4f8fa` | lit | **13.93** light end · **16.71** dark end |
| `#9aa1a6` | idle | **5.68** light end · **6.82** dark end |

### Accent

**One accent: `#ff2b1c`.** The engine lamps, the MOD ENGINE box's lamp and the scope trace. The
three engine cap colours are not accents — they are the hardware's own button colours and carry
no state.

---

## 7 · State matrices

### 7.1 Program legends — shared part

| Panel state | SAVE | STORE | DELETE | CANCEL |
|---|---|---|---|---|
| Factory Program, unmodified | idle | idle | idle | idle |
| Factory Program, edited | **lit** | idle | idle | idle |
| User Program, unmodified | idle | idle | **lit** | idle |
| User Program, edited | **lit** | idle | **lit** | idle |
| Naming a Program | idle | **lit** | idle | **lit** |

### 7.2 Pages and what dims

| Page | II | I | OFF cap | Knob group | Header · engine caps · footer · scope |
|---|---|---|---|---|---|
| I | lamp unlit | **lamp lit** | rest | full brightness | full brightness |
| II | **lamp lit** | lamp unlit | rest | full brightness | full brightness |
| I+II | **lamp lit** | **lamp lit** | rest | full brightness | full brightness |
| OFF | unlit | unlit | **pressed** | **brightness 0.5**, specular off, MOD ENGINE title to `#8a9196` | **unchanged — full brightness** |

**OFF dims the knob group and the lower boxes only.** The header, the three engine caps, the
footer and the scope stay lit: the unit is powered and passing dry signal, so darkening the
whole panel would claim it is off. This is the casting's departure from the suite's full-bleed
bypass multiply, and it is deliberate — OFF is a page, not a bypass.

### 7.3 IMAGE

| Image | Sprite |
|---|---|
| STEREO | `switch-stereo@2x.png` |
| MONO | `switch-mono@2x.png` |

Both legends printed in both states.

---

## 8 · Type

Every size is a CSS px em size with a pinned line box (call 4).

| Role | Face | Size / line box | Tracking | Ink |
|---|---|---|---|---|
| Wordmark | Librestile Extended | 28 / 32 | .02 em | `#e6ebee` |
| Function descriptor | Barlow Condensed 600 | 14 / 17 | .26 em | `#e6ebee` |
| Model line | Share Tech Mono | 11 / 14 | .20 em | `#b6bec2` |
| Box title | Barlow Condensed 600 | 12 / 15 | .28 em | `#e6ebee` |
| Scope title | Barlow Condensed 700 | 12 / 15 | .28 em | `#e6ebee` |
| Control label | Barlow Condensed 600 | 12 / 15 | .18 em | `#a5adb2` |
| Printed unit | Barlow Condensed 600 | 10 / 13 | .16 em | `#a5adb2` |
| Scale numeral | Share Tech Mono | 11 / 13 | 0 | `#a5adb2` |
| Engine legend | Barlow Condensed 700 | 22 / 22 · 18 / 18 | .10 em | see §2 |
| CHORUS badge | Barlow Condensed 700 | 26 / 30 | .34 em | `#dfe4ec` |
| IMAGE legend | Barlow Condensed 600 | 10 / 13 | .14 em | `#a5adb2` |
| Scope annotation · `250 ms / DIV` | Share Tech Mono | 11 / 14 | .06 em | `#9ba3a8` / `#8a9196` |
| LCD / meter value | Share Tech Mono | 17 / 22 | .10 em | `#dfe6ea` |
| Program legend | Barlow Condensed 600 | 11 / 13 | .12 em | see 7.1 |
| Footer | Share Tech Mono | 10 / 13 | .10 em | `#8a9196` |

**The 12 px scale numeral dropped to 11** under call 4. Numerals, the model line, the scope
annotations and the footer stay in Share Tech Mono — this casting's own mono, per call 7's
split; the wordmark is the nameplate metaphor and outside the call.

---

## 9 · Conformance — calls this casting already satisfied

**§9 and §10 together account for every call.** A call appearing in neither this section nor
the changelog is a gap by construction, not an omission.

| Call | State |
|---|---|
| **2** — Share Tech Mono LCD | **already conformed** on face; the cap moved up, 31 → 47. |
| **3's signature class** | **checked, and Chorus-60 takes no Ø104.** Its page selector is three buttons, not a detented dial, so the signature diameter would land on nothing. Two classes is the intended reading. |
| **5** — code-drawn, cached, no filmstrips | **already conformed** in artwork; its sheets are retired by the call. |
| **7** — Barlow Condensed panel lettering | **already conformed** for all panel lettering, with numerals in the casting's own mono. |
| **§4B** — multi-state control indicates state, legends printed once | **already conformed** by the IMAGE switch, which applies the rule to a sprite part rather than a drawn shoe: both legends permanent, the switch position carries the state. |
| **Registration** | **cannot apply** — no mixed-class row exists (§3). |
| **Lamps** — light stops at the lens edge, unlit stays in its own hue | **already conformed** on all four lamps. |

---

## 10 · Changelog and outstanding

### This round

1. **Canvas 1282 → 1340** (call 1), absorbed by the CHARACTER and OUTPUT boxes.
2. **Ø84 → Ø76 primary and Ø68 → Ø56 standard** (call 3); standard-class numerals cut to
   three with the demoted positions keeping their ticks.
3. **Scale numeral 12 px → 11 px** with a pinned line box (call 4).
4. **Header replaced by the shared part**; cap 31 → 47.
5. **LCD chevron re-drawn** as the shared 14 × 8 stroked path, replacing a 9 × 9 rotated box.
6. **Model line, PROGRAM and IN / OUT captions reconciled to `#b6bec2`** — the body carried
   `#8a9196` at 4.60 against its own header block, the worst functional figure found in the
   round; the six-material strip already had the corrected hex.
7. **Printed units `#8a9196` → `#a5adb2`**, 5.73 → 8.04, on the ground that a unit belongs to
   the printed scale rather than to the captions.
8. **Scope annotations drawn opaque** at `#9ba3a8` (7.52), replacing `rgba(160,178,186,.55)` at
   3.11.

### Outstanding

- ~~The fascia plate must be re-exported at 3×~~ — **exported, 4020 × 2436** from the current
  1340 × 812 canvas, not upscaled from the old 2564 × 1552.
- ~~Both IMAGE switch sprites must be re-cut at 3×~~ — **redrawn, 102 × 204 each**, from the
  casting's current inks in `Artwork Cutting Sheet.dc.html`. Their filenames still read `@2x`.
- Wire both meter wells and the scope to real signal; the render shows `−4.8` / `−3.1` as
  samples.
- Confirm RATE's skew and the four page pointer positions against the build's
  `NormalisableRange` before the ring is treated as final.
- **`shared/HEADER-PART.md` revision 3 is pending three build answers** — the meter's display
  clamp, its format at both ends, and the sign convention — plus a fourth item that is a
  process question rather than a figure: how a shared-part change reaches six bodies. Nothing
  on this panel changes either way.
