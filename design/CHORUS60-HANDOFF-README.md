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
2. `spec/CHORUS60-GUI-SPEC.md` — palette, type ramp, geometry, control coordinates, knob
   tick angles, parameter ranges and tapers, state machine.
3. `spec/BRAND.md` — wordmark, model-line lockup and colour rules.
4. `prototype/Chorus-60.dc.html` — open in a browser to interact with the real thing.

## Rules that carry the most risk
- **The plate carries text.** Printed scales, tick numerals, units, static section labels,
  the CHORUS stripe, model line and wordmark are all in the bitmap. Do not redraw them.
- **Runtime draws only**: mod-engine control labels, IMAGE I, page headings, button
  letters, scope trace, LCD contents, meter contents, footer status.
- **OFF state is a 0.50 multiply over the whole panel**, plate included — not a per-element
  restyle. The one exception is button letters, which are a live engaged/not readout.
- **Wells are baked, contents are not.** Scope, LCD (bank cell, divider, name field) and
  both meters ship as empty inset wells in the plate.

## Delta since the last spec revision — read this before the specs

**Knob filmstrips were re-rendered and the cap redesigned.** All four sheets ship (@1x and @2x, both
sizes), the frame box is now larger than the cap, and both pointers reach 0.520 D. The reference
renders were re-captured to match. Details in the "Knob filmstrips" section below and in
`spec/CHORUS60-BUILD-HANDOFF.md` §4 — that section supersedes anything older it contradicts.

The PROGRAM LCD now has a **chevron affordance** at the right edge of the name field:
11 × 7 px, 1.4 px stroke, square caps, `currentColor` at 0.75 opacity, vertically centred,
10 px inset from the field's right edge. The field gained 26 px of right padding to clear
it. It is hidden during name-entry and during parameter readouts — it appears only when the
LCD is showing a stored program, where it reads as "this is a picker." This is not yet
reflected in `CHORUS60-GUI-SPEC.md`; treat this README as the source for it.

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
- `controls/` — buttons (I / II / OFF), lamps (on / off), IMAGE switch as separate
  `switch-track` (34 × 26) and `switch-thumb` (26 × 26) sprites so the thumb animates a
  true 34 px travel rather than a crossfade. `switch-mono` / `switch-stereo` are the older
  whole-switch sprites, kept for reference. Each at @1x and @2x.
- `LibrestileExtBold.ttf` — display face for the wordmark and model line.

`reference/` — rendered panel states (Engine I, Engine II, I + II, OFF), captured from the current
prototype and matching the shipped knob art. **Review images only.** Do not use as build inputs or
slice from them.

## Parameter notes
Rate uses a JUCE `NormalisableRange` with **skew 0.35 over 0.05–16 Hz**. No lookup table.
Default program is factory index 0 (EIGHTY-TWO). All program names appearing in the spec
are examples, not a required factory bank.

## Files in this bundle
```
README.md                     this file
spec/CHORUS60-BUILD-HANDOFF.md   baked-vs-runtime split, asset manifest, export rules
spec/CHORUS60-GUI-SPEC.md        palette, type, geometry, coordinates, state machine
spec/BRAND.md                    wordmark and identity rules
prototype/Chorus-60.dc.html                   the interactive panel
prototype/Chorus-60 Background Plate.dc.html  plate generator
prototype/Chorus-60 Control Sprites.dc.html   sprite generator
prototype/Chorus-60 Icon.dc.html              product icon
prototype/support.js                          runtime for the prototypes
assets/                       production art
reference/                    review renders
```
Open any `.dc.html` directly in a browser; `support.js` must sit alongside them.
