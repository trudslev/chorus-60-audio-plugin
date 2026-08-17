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

The plate carries the fascia, the CHORUS badge and the box frames only. Knobs, lamps, labels,
ticks, numerals and the scope are drawn at runtime (GUI-SPEC §1).
