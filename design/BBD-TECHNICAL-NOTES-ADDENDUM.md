# Chorus-60 — BBD Technical Notes: I+II Correction

The original `BBD-TECHNICAL-NOTES.md` described I+II as "two genuinely independent
modulation processes running and summing." Based on a detailed real-circuit analysis,
this is incorrect and should be treated as superseded by this document.

Source: https://github.com/pendragon-andyh/Juno60/blob/master/Chorus/README.md

## What the real circuit actually does

There is only **one** LFO in the real circuit, not two independent engines. It
modulates two BBD delay lines (left and right), with the right channel's modulation
simply inverted 180°. That phase inversion is the entire stereo mechanism for Chorus 1
and Chorus 2 individually — much simpler than "differing phase/polarity per channel"
implied a more complex per-channel circuit than actually exists.

- **Chorus 1:** ~0.513 Hz, delay range ~1.66–5.35 ms, stereo
- **Chorus 2:** ~0.863 Hz, same delay range, stereo
- **Chorus 1+2 (both engaged): NOT a sum of the above.** It's a third, structurally
  distinct configuration: ~9.75 Hz (nearly 20x faster), a much narrower delay range
  (~3.3–3.7 ms), and the output collapses to **mono** on the real hardware — no phase
  inversion is applied in this mode. The source describes it as closer to a Leslie
  rotary speaker or vibrato than a chorus.

The source also notes real disagreement between Roland's own service documentation for
related models (Juno-60 service notes cite roughly 0.5/0.83/1 Hz; Juno-6 notes cite
roughly 0.4/0.67/8.06 Hz) — so treat the specific numbers as reference points rather
than a single ground truth. The structural insight is what matters and should be
treated as correct: **I+II is a distinct fast/narrow/mono mode, not an additive
combination of the I and II rates.**

## Implication for this plugin's architecture

Model the whole BBD chorus as **one reconfigurable engine with three parameter sets**
(I, II, I+II), not three parallel engines that can run simultaneously. This matches the
GUI's paged Mod Engine design — one box, one active configuration at a time, selected
by which physical button combination is engaged.

The plugin deliberately extends beyond hardware authenticity in one way: rather than
hard-locking I+II to mono, it exposes a Mono/Stereo switch on all three pages (I, II,
and I+II). I and II default to Stereo, I+II defaults to Mono — matching the real
circuit's authentic behavior as the default in every case — but I+II can be switched to
Stereo as a creative option the real hardware never offered. Decorrelation is a real,
always-available parameter on all three pages for the same reason: it has no audible
effect while Mono is selected, but becomes meaningful the moment Stereo is chosen.

## Consequences for the parameter layer

Two consequences follow from the above and are recorded here because both were wrong in
the pre-correction implementation.

**Delay Center and Decorrelation are per-configuration, not global.** The three
configurations have genuinely different delay ranges — I and II share ~1.66–5.35 ms
while I+II is the much narrower ~3.3–3.7 ms — so one shared Delay Center cannot express
them. They are now `center1` / `center2` / `centerB` and `decorr1` / `decorr2` /
`decorrB`. Drift, Saturation, Noise, Mix and Output Trim remain genuinely global.

**Rate I+II needs a wider range than Rate I/II.** The factory bank specifies I+II rates
of 9.75 Hz, 11 Hz and 14 Hz, all above the 0.05–8 Hz range that suits I and II. `rateB`
therefore spans 0.05–16 Hz while `rate1` and `rate2` keep 0.05–8 Hz. Sharing a single
range would silently clamp every I+II rate to 8 Hz and erase precisely the fast
character this document exists to describe.