# RE-CUT SHEET — CHORUS-60 CH-60

**Every row carries delivered *and* target dimensions.** A target dimension read without its
base is how three figures went wrong in this round: a needle height taken from a placement
offset, a plate "already 3×" of a canvas that no longer exists, a sprite "2×" against an old
frame. All three were true ratios with the base left out. This sheet exists so the target never
travels without it — `../MANIFEST.md` has the same rows for the whole suite.

| File | Drawn at 1× | Delivered | Ratio | **Target (3×)** |
|---|---|---|---|---|
| `assets/chorus60-background-plate@3x.png` | **1340 × 812** (current canvas) | **4020 × 2436** | 3× | **4020 × 2436** |
| `assets/switch-stereo@2x.png` — **REDRAWN, this bundle** | 34 × 68 | **102 × 204** | 3× | **102 × 204** |
| `assets/switch-mono@2x.png` — **REDRAWN, this bundle** | 34 × 68 | **102 × 204** | 3× | **102 × 204** |
| `assets/switch-track@3x.png` — **NEW, this call** | 34 × 68 | **102 × 204** | 3× | **102 × 204** |
| `assets/switch-thumb@3x.png` — **NEW, this call** | 26 × 26 | **78 × 78** | 3× | **78 × 78** |

**The plate was not a rescale.** Its old file was a correct 2× of the canvas this casting had
*before* call 1 — 1282 × 776 — and was re-exported from the current 1340 × 812 panel rather than
upscaled from 2564 × 1552, which would have been 3× of the wrong base.

**The two sprites are redrawn, not traced.** Their delivered bitmaps were a true 2× of a part
drawn before this round's ink pass, so tracing them would have carried the old inks back in. Both
states are now drawn in `Artwork Cutting Sheet.dc.html` at 34 × 68 — one pill body
`#191c1e → #0a0c0d` in a `#363c41` bezel, one Ø26 lens `#f6f9fb → #a8b0b5`, moved between top
and bottom — and cut from it at 3×. Both positions are printed alike; the shoe carries the state
(§4B).

**The `@2x` in both filenames is now stale** — the files are 3×. The names are the build's asset
paths, so they stay as they are until the build renames them; the figures in this sheet, not the
filename, say what scale each file is.

## The switch, as two parts

The composites stay as the reference for what the parts must add up to; they were not re-cut. The
same drawing is now also delivered split, because §4B makes the shoe the indicating mechanism and a
pre-baked pair would remove the indication:

- **`switch-track@3x.png`** — the pill body in its `#363c41` bezel, no lens, 34 × 68 → 102 × 204.
- **`switch-thumb@3x.png`** — the Ø26 lens alone on transparency, 26 × 26 → 78 × 78.

Composite the thumb at `left: 4px` with `top: 4px` for STEREO and `top: 38px` for MONO. Ø26, 34 px
of travel, 4 px inset at each end, in a 34 × 68 body — the geometry is unchanged from the
composites and from the build's own constants.

**The cast shadow ships on the thumb**, drawn inward so it fits the 26 × 26 box: the composites'
`0 2px 4px rgba(0,0,0,.6)` drop is a bottom-weighted inset of the same value in the cut part. It
cannot live on the track — a static body cannot shade a lens that moves — and at Ø26 the two read
alike. The track keeps only its own `inset 0 2px 5px rgba(0,0,0,.85)` well.

Inks in both parts are this round's, matching the composites: bezel `#363c41`, body
`#191c1e → #0a0c0d` **darkening** downward, lens `#f6f9fb → #a8b0b5`. The parts the build ships
today differ on all three — bezel absent, lens ramp short at both ends, body gradient running the
opposite direction — so they are replaced outright, not adjusted.

The plate carries the fascia, the CHORUS badge and the box frames only. Knobs, lamps, labels,
ticks, numerals and the scope are drawn at runtime (GUI-SPEC §1).
