# Handoff: CHORUS-60 (CH-60) plugin interface

## Overview
A full audio-plugin front panel: a fixed-size hardware-style GUI with two mod engines
(I and II), a shared global section, a scope, a PROGRAM LCD with browse/store/name
behaviour, IN/OUT meters, and a stereo IMAGE switch. This bundle is everything needed to
build the panel in the plugin host framework (JUCE is the assumed target).

## About the design files
The `.dc.html` files in `prototype/` are **design references created in HTML** —
interactive prototypes that show the intended look and behaviour. They are not production
code to port. The task is to **recreate the design in the plugin codebase's own
environment** (JUCE `Component`/`LookAndFeel`, or whatever the project already uses),
following its established patterns.

The prototype is authoritative for *behaviour and pixel geometry*; the two spec documents
are authoritative for *values*. Where they disagree, the spec wins — file a note.

## Fidelity
**High fidelity.** Final colours, typography, spacing, sprite geometry and interaction
timing. Build pixel-perfect against the spec. All coordinates in the spec are @1x panel
points; multiply by 2 for the @2x asset set.

## Where to start
1. `spec/CHORUS60-BUILD-HANDOFF.md` — **read §1 first.** It defines the split between what
   is baked into the background bitmap and what the code draws at runtime. Getting this
   wrong is the single most expensive mistake available here (double-drawn labels,
   drifting scales).
2. `spec/GUI-SPEC.md` — **the build contract.** Palette, type ramp, geometry, control
   coordinates, knob tick angles, parameter ranges and tapers, state machine. Its opening lines name
   the one companion document (`CHORUS60-BUILD-HANDOFF.md`, the asset contract), so you never have to
   read the bundle to find out which file to build from.
3. `BRAND.md` is **not** in this bundle, on purpose. It is repo-owned and it moves — a copy cut this
   morning contradicts the repo by lunchtime. Read it from the repo; the header-row, Program-button
   and legibility-floor rules this revision implements live there.
4. `prototype/Chorus-60.dc.html` — open in a browser to interact with the real thing.

## Rules that carry the most risk
- **The plate carries text.** Printed scales, tick numerals, units, static section labels,
  the CHORUS stripe, model line and wordmark are all in the bitmap. Do not redraw them.
- **Runtime draws only**: mod-engine control labels, IMAGE I, page headings, button
  letters, scope trace, LCD contents, meter contents, footer status.
- **OFF state is a 0.50 multiply over the whole panel**, plate included — not a per-element
  restyle. The one exception is button letters, which are a live engaged/not readout.
- **Wells are baked, contents are not.** Scope, LCD (bank cell, divider, name field) and
  both meters ship as empty inset wells in the plate. **The two Program buttons ship the same
  way** — empty dark boxes; all four legends are drawn at runtime because their colour is state.
- **The Program buttons never relabel.** `SAVE`/`STORE` and `DELETE`/`CANCEL` are both always
  printed; only which one is lit changes. There is no disabled face and no greyed-out button.

## Delta since the last spec revision — read this before the specs

**Asset completeness pass (this revision).** The eight-file product-icon ladder was listed in
`CHORUS60-BUILD-HANDOFF.md` §5 but had never been bundled — it is now in `assets/icon/`, with
`prototype/Chorus-60 Icon.dc.html` as its source. The six Program-button faces shipped @2x only while
every other control sprite ships both; their @1x exports are now in `assets/controls/`. A previous
note claiming the engine buttons, lamps and IMAGE switch had never been exported was **wrong** —
all nine have been present at @1x and @2x throughout. No design changed.


**Bundle re-cut to the delivery convention (this revision).** Three changes, no design impact:
the build contract is now `spec/GUI-SPEC.md` (was `CHORUS60-GUI-SPEC.md`) and its opening lines name
its one companion so you never have to read the bundle to find the contract; `BRAND.md` is no longer
bundled — it is repo-owned and moves, so a bundled copy would read as authoritative while being
stale; and §13's Program-button table now carries a second table of the **behavioural** rules the
appearance table cannot imply (Escape out of naming leaves the Program modified, the bank tag reads
`NAME` until STORE commits, empty names commit as `NEW PROGRAM`, and what sets and clears the edited
flag). Reference renders of the button pair as one band, one per state, are in `reference/buttons/`.


**Program-button conformance pass against BRAND.md (this revision).** Checked the two buttons against
every Program-button rule in the brand document. They pass on structure — two permanent stacked
legends, no relabelling, dark face on both, both legends stepped back for "nothing to do", no
disabled/recessed face and no fourth sprite, no lamp or bezel element, 10 px legends, the shared 34 px
height, lit ink that is not the accent, SAVE lit off the same flag as the LCD `*` marker, DELETE lit
only on User Programs.

One rule failed and is fixed: the bloom was **cool** blue-white, where the brand specifies a warm
bloom — the thing that makes the legend read as backlit rather than as brighter ink. Now
`rgba(255,229,188,.5)`, with the lit ink moved `#EEF3F6` → `#F1EFEA` so cool letterforms don't sit
inside a warm glow. All six `program-*@2x.png` faces and the naming reference render were re-exported.

Also corrected: the unlit legend was published at **3.95:1**, which is the midpoint of the face
gradient. Worst case at the lightest part of the face is **3.55:1** — still clear of the floor, but the
specs now quote the worst case, since a midpoint is the wrong number for a check to trust.


**Header row rebuilt to the suite-wide 34 px standard.** LCD, SAVE, DELETE, IN and OUT now share one
baseline (y 31) and one outer height (34 px) exactly — border-box, so the wells' 1 px borders no
longer make them 2 px taller than the buttons. The 6 px came out of the header band's own padding
(16 / 14 → 14 / 12), so the band stays 78 px and **nothing in the body moved**. Meters widened 54 → 56
so their borders sit inside the stated box.

**The Program buttons became dual stacked legends with backlit state.** SAVE above STORE, DELETE
above CANCEL, permanently printed; the live function is lit `#F1EFEA` with a soft **warm** bloom, the other
sits matte at `#757D82` with no shadow at all (3.55:1 worst case, above the 3:1 state floor). The disabled face is gone. The button
faces changed from pale to dark to carry backlighting — a pale face has nowhere brighter to go, which
is why that form uses a lamp beside the legend instead. The bloom is warm and the ink near-neutral: what separates lit from unlit has to be
*luminous vs matte*, not two strengths of the same ink. The accent red stays with the engine lamps and
appears nowhere on these buttons. Full state table, plus the behavioural rules the appearance table cannot imply, in `spec/GUI-SPEC.md` §13.

**LCD character budget confirmed unchanged: 36 characters, 31-character user-name cap.** The height
change did not touch the name field's width, face, size or tracking. The prototype was capping typed
names at 24 — below the published cap — and is corrected to 31. This cap may never be reduced.

The plate and all four reference renders were re-exported from the current panel.

**Knob filmstrips were re-rendered and the cap redesigned.** All four sheets ship (@1x and @2x, both
sizes), the frame box is now larger than the cap, and both pointers reach 0.520 D. The reference
renders were re-captured to match. Details in the "Knob filmstrips" section below and in
`spec/CHORUS60-BUILD-HANDOFF.md` §4 — that section supersedes anything older it contradicts.

The PROGRAM LCD now has a **chevron affordance** at the right edge of the name field:
11 × 7 px, 1.4 px stroke, square caps, `currentColor` at 0.75 opacity, vertically centred,
10 px inset from the field's right edge. The field gained 26 px of right padding to clear
it. It is hidden during name-entry and during parameter readouts — it appears only when the
LCD is showing a stored program, where it reads as "this is a picker." This is not yet
reflected in `GUI-SPEC.md`; treat this README as the source for it.

## Knob filmstrips — re-rendered, and the cap look changed
All four sheets ship: `@1x` and `@2x`, both knob sizes, each rendered natively at its own size by
the parametric generator. Nothing is upscaled and retina is no longer an open dependency.

**Two things changed that the build must account for.**

The **frame box is now larger than the cap** — Ø84 in a 112 box, Ø68 in a 92 box, doubled at `@2x`.
The margin is where the cast shadow fades to zero, so do not crop it, and **centre each knob on the
cap, not on the frame box**. Treating the box as the knob renders every control ~33 % oversized. The
previous cap-fills-frame sheets clipped the shadow square at the frame edge; these pass the
generator's shadow guard.

The **cap itself looks different** — lighter body, flatter falloff, knurl reaching further in. The
knob is shared with Gatecrasher and this brings Chorus-60 onto its current render, so the two plugins
stay on one design. It is a visible panel change: the renders in `reference/` predate it.

The generator ships with the bundle — `tools/render-knob-filmstrips-chorus60.mjs` (`npm i canvas`,
then `node render-knob-filmstrips-chorus60.mjs <outDir>`). It regenerates all four sheets and checks
that the cast shadow reaches zero inside every frame box.

Full detail in `spec/CHORUS60-BUILD-HANDOFF.md` §4.

## Assets
`assets/`
- `chorus60-background-plate@1x.png`, `@2x.png` — the panel plate. Generated from
  `prototype/Chorus-60 Background Plate.dc.html`, which is the same markup as the panel
  with runtime strings hidden, so plate and panel cannot drift. Regenerate from that file,
  never by hand.
- `knob_mod_84px_128f.png` (Ø84 cap / 112 box), `knob_global_68px_128f.png` (Ø68 / 92) — @1x
  vertical strips.
- `knob_mod_168px_128f@2x.png` (Ø168 / 224), `knob_global_136px_128f@2x.png` (Ø136 / 184) — @2x,
  8 × 16 row-major grids.
- `controls/` — **currently holds only the six `program-*@2x.png` faces exported this revision.**
  The engine buttons, lamps and IMAGE switch have never been exported to PNG; their source is
  `prototype/Chorus-60 Control Sprites.dc.html` and the build has been reading geometry from the spec.
  Same for `assets/icon/` — the ladder is described in the specs but not present in this bundle.
  Flag if you need either exported.
- (original note) buttons (I / II / OFF), lamps (on / off), IMAGE switch as separate
  `switch-track` (34 × 26) and `switch-thumb` (26 × 26) sprites so the thumb animates a
  true 34 px travel rather than a crossfade. `switch-mono` / `switch-stereo` are the older
  whole-switch sprites, kept for reference. Each at @1x and @2x.
- `LibrestileExtBold.ttf` — display face for the wordmark and model line.

`reference/` — rendered panel states (Engine I, Engine II, I + II, OFF) plus
`chorus60-header-naming@2x.png` (the header row during name entry, showing STORE and CANCEL lit),
captured from the current prototype and matching the shipped knob art. **Review images only.** Do not use as build inputs or
slice from them.

## Parameter notes
Rate uses a JUCE `NormalisableRange` with **skew 0.35 over 0.05–16 Hz**. No lookup table.
Default program is factory index 0 (EIGHTY-TWO). All program names appearing in the spec
are examples, not a required factory bank.

## Files in this bundle
```
README.md                     this file
spec/GUI-SPEC.md                 THE BUILD CONTRACT — palette, type, geometry, coordinates, states
spec/CHORUS60-BUILD-HANDOFF.md   companion: baked-vs-runtime split, asset manifest, export rules
reference/buttons/               the Program-button pair rendered per state, 3x
prototype/Chorus-60.dc.html                   the interactive panel
prototype/Chorus-60 Background Plate.dc.html  plate generator
prototype/Chorus-60 Control Sprites.dc.html   sprite generator
prototype/Chorus-60 Icon.dc.html              product icon
prototype/support.js                          runtime for the prototypes
assets/                       production art
reference/                    review renders
```
Open any `.dc.html` directly in a browser; `support.js` must sit alongside them, and
`prototype/assets/LibrestileExtBold.ttf` is the wordmark face the panel loads by relative path —
without it the wordmark silently falls back to Barlow Condensed.
