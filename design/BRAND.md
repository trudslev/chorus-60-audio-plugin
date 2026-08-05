# Neon Foundry — Brand & Design System

This document is the shared reference for every plugin under the Neon Foundry umbrella.
Individual plugins (TapeRot, Gatecrasher, and whatever follows) each get their own
distinct visual identity — but they share a common DNA. This file is that DNA:
the rules that stay constant so a new plugin feels like family without copying
an existing one wholesale.

When starting or extending any plugin in this suite, read this file first, then
read that plugin's own `design/` folder and GUI source for its specific execution
of these rules.

## Philosophy

Neon Foundry plugins emulate the *feeling* of specific eras and classes of
outboard hardware — not a single shared skin. Each plugin should look like it
could plausibly have shipped from a real 1980s/90s hardware manufacturer, with
its own fascia color, its own material logic, its own nameplate treatment —
chosen because it fits that plugin's function, not because it matches its
siblings.

What ties them together instead:
- The same underlying *component grammar* (see below) — how knobs, LEDs, and
  meters are built and behave, even when their finish differs
- The same commitment to specificity — real hardware spec-sheet language,
  plausible model numbers, functional-not-decorative detail
- The same restraint — one signature visual element per plugin, everything
  else quiet and disciplined around it
- A shared naming and terminology convention (see below)

"Old process, new signal" — hand-forged craft, cast in electric material.

## Naming & terminology

- The umbrella brand is **Neon Foundry**.
- Individual plugins are internally called **castings** in marketing copy
  (e.g. "the foundry's third casting") — this is copy language, not a UI label.
- Stored parameter snapshots are called **Programs**, never "Presets" — this
  matches period-correct hardware terminology (Lexicon 224, dbx, AMS-era rack
  units all called them programs; "preset" is the modern software-plugin term
  we're deliberately avoiding). This applies to the UI label, the parameter
  name, factory bank documentation, and marketing copy alike.
- Plugin GUIs favor real hardware-spec-sheet phrasing over software-plugin
  phrasing: a model tagline ("MODEL GR-85 · STEREO"), a one-line function
  descriptor in the hardware's own voice ("GATED AMBIENCE PROCESSOR"), a
  version stamp in the corner (v1.0) — the plugin should read like it came
  with a printed manual.

## Shared component grammar

These behaviors and structures should be consistent across every plugin,
even when their visual finish differs:

- **Knobs**: sculpted body, tick marks around the travel arc, a single clean
  pointer line indicating value. Chunkier/larger for primary character
  controls, smaller for secondary/tone-shaping controls — knob size should
  communicate importance.
- **LEDs**: a single dedicated LED (or LED-style lamp) reporting the most
  important live discrete state of that plugin — TapeRot's `FailLamp` for
  failure-engine state, Gatecrasher's "GATE OPEN" lamp for trigger state.
  Every plugin should have exactly one of these. It gets exactly one accent
  color, and that color is reserved *only* for that state — never reused
  elsewhere on the same panel for anything decorative.
- **Live scopes/meters**: where a plugin has a process worth watching in real
  time (tape wow/flutter, gate envelope), give it a real oscilloscope-style
  readout — grid lines, a division-rate label (e.g. "50 ms / DIV"), and where
  relevant, the underlying audio/waveform rendered subtly behind the primary
  trace so the display reads as a real diagnostic tool, not decoration.
- **Nameplate**: every plugin gets a distinct physical-application metaphor
  for its wordmark — TapeRot's hand-applied Dymo label, Gatecrasher's
  distressed stencil/spray treatment. Pick the metaphor that fits *that*
  plugin's function; don't reuse one plugin's nameplate style on another.
- **Fixed-aspect canvas**: plugin GUIs are designed to a fixed aspect ratio,
  960×400 as the reference (TapeRot's canvas). Plugins with a denser control
  set may scale proportionally, but should stay close to this ratio rather
  than introducing a new shape per plugin.
- **Numeric readouts**: prefer real metering language over vague labels —
  actual dB values on input/output meters, actual ms/Hz/% values under knobs,
  not just knob position with no numeric confirmation.

## Color system

There is no single shared Neon Foundry palette — each plugin's fascia color
is a deliberate choice tied to its era/character (TapeRot: warm and worn;
Gatecrasher: cold steel or period-correct light grey/silver rack-unit finish).
What *is* shared is the rule, not the hex values:

- Exactly one accent color per plugin, reserved exclusively for that plugin's
  single most important live state indicator (the LED + its associated live
  trace, e.g. Gatecrasher's red gate LED and gate-envelope trace).
- That accent color is never used elsewhere on the same panel — not on other
  knobs, not on section labels, not as general decoration. If it appears
  anywhere else, it dilutes the one moment it's supposed to draw the eye to.
- Everything else on the panel — knob bodies, general labels, secondary
  meters — stays in a neutral grey/steel/white range appropriate to that
  plugin's chosen base material.

## Typography

- Plugin wordmark: a bold, characterful display treatment specific to that
  plugin's nameplate metaphor (label-maker font for TapeRot, distressed
  stencil for Gatecrasher) — never a neutral system sans for the main title.
- Functional tagline / model line beneath the wordmark: small, restrained,
  wide letter-spacing, real hardware-manual voice.
- Numeric/LCD-style readouts (program name, meter values): a monospace or
  segment-display face, distinct from the body label typeface, reinforcing
  "this is a real digital readout" rather than a UI label.
- Section and control labels: small, neutral, wide letter-spacing, low-key —
  these should never compete with the wordmark or the signature element.

## Roster

| Plugin | Format | Character | Status |
|---|---|---|---|
| **TapeRot** | AU/VST3/Standalone | Warm, worn, analog decay | Shipping — all DSP stages implemented |
| **Gatecrasher** | AU/VST3/Standalone | Cold, aggressive, gated reverb | In design — GUI mockup approved, DSP/build pending |
| **CHORUS-60** | AU/VST3/Standalone | Near-black synth panel, BBD chorus, dual mod engines | In design — GUI mockup approved, DSP/build pending |

Update this table as new castings are started or shipped.

## Folder structure

```
neon-foundry/
  BRAND.md                 This file
  taperot/
    Source/, Tests/, design/, BUILDING.md, prompts/
  gatecrasher/
    Source/, Tests/, design/, BUILDING.md, prompts/
  <next-plugin>/
    ...
```

Giving Claude Code root-level access to `neon-foundry/` (rather than one
plugin folder at a time) lets it read this file plus any sibling plugin's
`design/` and GUI source when making design decisions for a new casting —
so new plugins can share DNA with what's already shipped without repeating
instructions from scratch each time.