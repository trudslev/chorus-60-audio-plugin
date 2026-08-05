# CHORUS-60 CH-60 — GUI handoff

You are building the JUCE plugin GUI for **CHORUS-60**, a Juno-60 / JN-80 style BBD chorus and the
third plugin in the same suite as **Gatecrasher GR-85** and **TapeRot**. The visual design is
approved and final — implement it, don't redesign it.

Read `CHORUS60-GUI-SPEC.md` first. It has the palette, every control coordinate, the filmstrip
contract, the header/program contract shared with Gatecrasher, and the scope drawing rules.

**Assets**
- `assets/chorus60-panel@2x.png` — approved reference, engine I engaged (default program)
- `assets/chorus60-panel-both-engines@2x.png` — I + II engaged (both LEDs lit)
- `assets/chorus60-panel-bypass@2x.png` — OFF (both LEDs dark, scope at drift floor)
- `assets/knob_large_128px_128f.png` — 128 frames, knurled, −135°→+135° (**shared with Gatecrasher**)
- `assets/knob_small_128px_128f.png` — 128 frames, plain skirt (**shared with Gatecrasher**)
- `assets/header-factory-program@3x.png` — header, factory program (DELETE disabled)
- `assets/header-user-program@3x.png` — header, user program (DELETE enabled)
- `assets/header-name-entry@3x.png` — header, SAVE pressed / naming a program
- `assets/jn80-chorus-reference.jpeg` — photo of the real hardware chorus section. This is the
  authority for the button colours, the LED, the blue section stripes and the panel blackness.
- `assets/gatecrasher-panel-reference.png` — the approved Gatecrasher panel, for suite grammar
  (header, LCD, meter windows, knob rendering, scope treatment).
- Font `LibrestileExtBold.ttf` is used for the wordmark only. Unlike Gatecrasher, **this wordmark
  is clean type, not a baked spray render** — see §8.

**Live reference**
`reference/Chorus-60.dc.html` is the working mockup — open it in a browser to see the scope
animating, the engine buttons latching, and the SAVE / DELETE / name-entry flow behaving.
`reference/support.js` and `reference/assets/` must sit alongside it (they do in this package).

Read the mockup's source for exact gradients, shadows and the scope draw loop — every value in the
spec came from it. It uses inline CSS on a small custom runtime; treat it as a visual and
behavioural reference, not as code to port.

**Decisions already made**
- Knobs are **filmstrips**, not code-drawn — the same two strips Gatecrasher ships. Rendered at
  128px so all panel sizes downscale.
- Tick rings are drawn in code, separately from the strips (they don't rotate).
- Panel is fixed **1400 × 632** at 1× — 2.22:1, the same canvas ratio as Gatecrasher (BRAND.md).
- Single page. No tabs, no pages, no hidden controls — everything is visible at once.
- The chassis is near-black synth-panel material, deliberately unlike Gatecrasher's light steel rack
  and TapeRot's warm tape-deck fascia. There are no rack ears and no screws: this panel reads as if
  it were unbolted out of a synthesizer.
- The two blue stripes framing the button column are **structural**, straight off the hardware.
  They are the only chrome colour on the panel and they are not an accent — don't add more blue
  anywhere else.

**Program management** (§6) — identical contract to Gatecrasher: stamped-steel SAVE / DELETE right
of the LCD, DELETE disabled on factory programs, read-only FACT / USER tag cell inside the LCD,
SAVE switching the LCD into name-entry with a blinking caret and relabelling the buttons
STORE / CANCEL.

**Engine buttons** (§4) — three square hardware buttons: II (orange), I (yellow/tan), OFF (white).
I and II latch independently; either, both, or neither may be engaged. OFF clears both. Each engine
button has its own red LED; OFF has none.

**Brand**
This plugin is a Neon Foundry casting — read `BRAND.md` in the suite root alongside this file.
§12 of the spec records the audit against it, including the three deliberate deviations (two engine
LEDs, the hardware button colours, the blue section stripes). Don't "correct" those.

**Non-negotiable**
- Red `#FF2B1C` only on the two engine LEDs and the scope trace. Never tint a knob, label, meter,
  or LCD red. There is no separate lamp above the scope — BRAND.md allows one live-state indicator
  per panel and the engine LEDs are it.
- LEDs light on the same sample the engine engages — no fade-in.
- The scope shows the *actual* delay-modulation signal, not a decorative sine. When the parameters
  move, the trace moves with them on the next frame.
- Sharp corners everywhere on the fascia, LED windows, group panels and stripes. The only rounded
  things on the panel are the three engine buttons (5px, as on the hardware), the knobs and the LEDs.
- No suite/brand name anywhere on the panel — wordmark and model line only.
