# RE-CUT SHEET — CHORUS-60 CH-60

**Every row carries delivered *and* target dimensions.** A target dimension read without its
base is how three figures went wrong in this round: a needle height taken from a placement
offset, a plate "already 3×" of a canvas that no longer exists, a sprite "2×" against an old
frame. All three were true ratios with the base left out. This sheet exists so the target never
travels without it — `../MANIFEST.md` has the same rows for the whole suite.

| File | Drawn at 1× | Delivered | Ratio | **Target (3×)** |
|---|---|---|---|---|
| `assets/chorus60-background-plate@2x.png` | **1340 × 812** (current canvas) | 2564 × 1552 | 2× of **1282 × 776** — superseded | **4020 × 2436** |
| `assets/switch-stereo@2x.png` | 34 × 68 | **68 × 136** | 2× | **102 × 204** |
| `assets/switch-mono@2x.png` | 34 × 68 | **68 × 136** | 2× | **102 × 204** |

**The plate is not a rescale.** Its delivered file is a correct 2× of the canvas this casting had
*before* call 1 — 1282 × 776. Re-export from the current 1340 × 812 panel; do not upscale
2564 × 1552, which would be 3× of the wrong base.

The plate carries the fascia, the CHORUS badge and the box frames only. Knobs, lamps, labels,
ticks, numerals and the scope are drawn at runtime (GUI-SPEC §1).
