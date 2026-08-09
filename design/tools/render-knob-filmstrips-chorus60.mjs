// Chorus-60 CH-60 — knob filmstrip generator
//
// Derived from Gatecrasher's render-knob-filmstrips.mjs, with Chorus-60's
// diameters and two deliberate differences (see CHANGES below).
//
// Produces the four sheets shipped in assets/:
//   knob_mod_84px_128f.png         112 x 14336    cap 84,  knurled, vertical strip
//   knob_global_68px_128f.png       92 x 11776    cap 68,  plain,   vertical strip
//   knob_mod_168px_128f@2x.png    1792 x 3584     cap 168, knurled, 8 x 16 grid
//   knob_global_136px_128f@2x.png 1472 x 2944     cap 136, plain,   8 x 16 grid
//
// Every sheet is 128 frames, frame 0 = -135 deg, frame 127 = +135 deg.
// Each is rendered natively at its own size — nothing is upscaled.
//
// The frame box is LARGER than the cap (ratio 0.75; global 0.739, because
// 68 / 0.75 is not an integer and 92 keeps @2x an exact doubling). The margin
// is not padding: it is where the cast shadow fades to zero. Do not re-crop,
// and position knobs from the CAP centre, not the frame box.
//
// CHANGES vs the Gatecrasher generator:
//   1. Darker cap, lit from the centre. Gatecrasher lights the cap off-axis
//      from the upper left, which read grey and flat against Chorus-60's dark
//      fascia. Chorus-60 uses a CENTRED radial falloff instead — grey in the
//      middle, black at the rim, matching the original CH-60 knob. Gradient is
//      centred on the cap with radius 0.62 x D and stops #5a626a / #2d333a /
//      #0e1114 / #030405; top highlight .13 -> .06, bottom occlusion .28 -> .18
//      (the vignette now does that work), knurl shadow band .18 -> .22.
//   2. The knurl rotates with the frame. Gatecrasher draws the skirt at fixed
//      angles, so only the pointer moves and the flutes sit still while the
//      knob turns. Here the serration offset is advanced by angleDeg.
//   3. Both pointers reach the cap centre. Gatecrasher's two strips disagree:
//      the small one is thick and overshoots (pointerW 0.075, tip at 0.125 +
//      0.500 = 0.625 D) while the large one stops short at 0.468 D. Both now
//      land at 0.520 D — global pointerW 0.056 / top 0.115 / len 0.405, mod
//      pointerW 0.048 / top 0.081 / len 0.439.
//
//   npm i canvas
//   node render-knob-filmstrips-chorus60.mjs [outDir]

import { createCanvas } from 'canvas';
import { writeFileSync, mkdirSync } from 'node:fs';
import { join } from 'node:path';

const FRAMES = 128;
const SWEEP = 270;        // degrees, symmetric about 12 o'clock
const REF_BOX = 160;      // the box the shadow/stroke constants were authored against

// name, cap diameter, frame box, knurled, layout, pointer geometry (as
// fractions of cap diameter)
const SHEETS = [
  { file: 'knob_mod_84px_128f.png',          cap: 84,  box: 112, knurled: true,  cols: 1, pointerW: 0.048, pointerTop: 0.081, pointerLen: 0.439 },
  { file: 'knob_global_68px_128f.png',       cap: 68,  box: 92,  knurled: false, cols: 1, pointerW: 0.056, pointerTop: 0.115, pointerLen: 0.405 },
  { file: 'knob_mod_168px_128f@2x.png',      cap: 168, box: 224, knurled: true,  cols: 8, pointerW: 0.048, pointerTop: 0.081, pointerLen: 0.439 },
  { file: 'knob_global_136px_128f@2x.png',   cap: 136, box: 184, knurled: false, cols: 8, pointerW: 0.056, pointerTop: 0.115, pointerLen: 0.405 },
];

function drawKnob(ctx, cx, cy, R, angleDeg, o, S) {
  const D = 2 * R;
  const pw = o.pointerW * D, ptop = o.pointerTop * D, plen = o.pointerLen * D;

  // 1. cast shadow — an opaque disc drawn only for its shadow, overpainted next
  ctx.save();
  ctx.shadowColor = 'rgba(0,0,0,.55)';
  ctx.shadowBlur = 11 * S;
  ctx.shadowOffsetY = 8 * S;
  ctx.beginPath();
  ctx.arc(cx, cy, R, 0, Math.PI * 2);
  ctx.fillStyle = '#0b0d0f';
  ctx.fill();
  ctx.restore();

  ctx.save();
  ctx.beginPath();
  ctx.arc(cx, cy, R, 0, Math.PI * 2);
  ctx.clip();

  // 2. cap body — centred radial falloff: grey core, black rim
  const g = ctx.createRadialGradient(cx, cy, 0, cx, cy, 0.62 * D);
  g.addColorStop(0, '#5a626a');
  g.addColorStop(0.42, '#2d333a');
  g.addColorStop(0.75, '#0e1114');
  g.addColorStop(1, '#030405');
  ctx.fillStyle = g;
  ctx.fillRect(cx - R, cy - R, D, D);

  // 3. knurled rim — 60 serrations, fixed 6 deg step at every scale, advanced
  //    by the frame angle so the skirt turns with the knob.
  if (o.knurled) {
    const inner = R * 0.86;
    for (let a = 0; a < 360; a += 6) {
      const bands = [
        [0,   2.5, 'rgba(255,255,255,.07)'],
        [2.5, 3.5, 'rgba(0,0,0,.22)'],
      ];
      for (const [off, w, col] of bands) {
        const a0 = (a + off + angleDeg) * Math.PI / 180;
        const a1 = (a + off + w + angleDeg) * Math.PI / 180;
        ctx.beginPath();
        ctx.arc(cx, cy, R, a0, a1);
        ctx.arc(cx, cy, inner, a1, a0, true);
        ctx.closePath();
        ctx.fillStyle = col;
        ctx.fill();
      }
    }
  }

  // 4. top highlight / bottom occlusion
  const h = ctx.createLinearGradient(0, cy - R, 0, cy + R);
  h.addColorStop(0, 'rgba(255,255,255,.06)');
  h.addColorStop(0.4, 'rgba(255,255,255,0)');
  h.addColorStop(0.85, 'rgba(0,0,0,0)');
  h.addColorStop(1, 'rgba(0,0,0,.18)');
  ctx.fillStyle = h;
  ctx.fillRect(cx - R, cy - R, D, D);

  // 5. pointer, rotated to this frame's angle
  ctx.translate(cx, cy);
  ctx.rotate(angleDeg * Math.PI / 180);
  const pg = ctx.createLinearGradient(0, -R + ptop, 0, -R + ptop + plen);
  pg.addColorStop(0, '#ffffff');
  pg.addColorStop(1, '#c2c8cd');
  ctx.shadowColor = 'rgba(255,255,255,.30)';
  ctx.shadowBlur = 5 * S;
  ctx.fillStyle = pg;
  ctx.fillRect(-pw / 2, -R + ptop, pw, plen);
  ctx.restore();

  // 6. rim stroke, inside the clip edge so it stays crisp
  ctx.beginPath();
  ctx.arc(cx, cy, R - 1 * S, 0, Math.PI * 2);
  ctx.strokeStyle = '#08090a';
  ctx.lineWidth = 2 * S;
  ctx.stroke();
}

function renderSheet(s) {
  const rows = FRAMES / s.cols;
  const canvas = createCanvas(s.box * s.cols, s.box * rows);
  const ctx = canvas.getContext('2d');
  const S = s.box / REF_BOX;
  for (let i = 0; i < FRAMES; i++) {
    const angle = -SWEEP / 2 + i * SWEEP / (FRAMES - 1);
    ctx.save();
    ctx.translate((i % s.cols) * s.box, Math.floor(i / s.cols) * s.box);
    drawKnob(ctx, s.box / 2, s.box / 2, s.cap / 2, angle, s, S);
    ctx.restore();
  }
  return canvas;
}

// Guard: the shadow must fade to zero inside the frame at every angle. The
// retired cap-fills-frame sheets failed this at 88 top / 95 bottom / 38 sides.
function maxBorderAlpha(canvas, s) {
  const ctx = canvas.getContext('2d');
  let worst = 0;
  for (const fi of [0, 32, 64, 96, 127]) {
    const ox = (fi % s.cols) * s.box;
    const oy = Math.floor(fi / s.cols) * s.box;
    const d = ctx.getImageData(ox, oy, s.box, s.box).data;
    const A = (x, y) => d[(y * s.box + x) * 4 + 3];
    for (let i = 0; i < s.box; i++) {
      worst = Math.max(worst, A(i, 0), A(i, s.box - 1), A(0, i), A(s.box - 1, i));
    }
  }
  return worst;
}

const outDir = process.argv[2] ?? '.';
mkdirSync(outDir, { recursive: true });

for (const s of SHEETS) {
  const canvas = renderSheet(s);
  writeFileSync(join(outDir, s.file), canvas.toBuffer('image/png'));
  const worst = maxBorderAlpha(canvas, s);
  console.log(
    `${s.file}  ${canvas.width} x ${canvas.height}  cap ${s.cap} in ${s.box}  border alpha ${worst}` +
    (worst > 2 ? '  <-- SHADOW IS CLIPPED, raise the frame box' : '')
  );
}
