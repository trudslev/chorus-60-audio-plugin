# CHORUS-60

A Juno-60/JN-80-style BBD ensemble chorus plugin (AU/VST3/Standalone on macOS; VST3/Standalone on
Windows/Linux), built with JUCE 8. Third casting under the [Neon Foundry](../BRAND.md) umbrella,
sibling to [TapeRot](../taperot/) and [Gatecrasher](../gatecrasher/).

CHORUS-60 emulates a real bucket-brigade-delay ensemble chorus: two independent modulation engines
(each a rounded/asymmetric LFO driving a short variable delay tap, never a pure sine) read from a
shared BBD delay line with a fixed reconstruction-filter rolloff, decorrelated across L/R for real
stereo width rather than panning, then given the small nonlinear "character" a real BBD circuit has
(drift, saturation, noise floor) before the final dry/wet/trim stage. Engine I and Engine II are
independent switches - engaging both together is the classic Juno "I+II" trick, not a third blended
mode.

## Parameters

| Parameter | Range | Default | Notes |
|---|---|---|---|
| Chorus I | on/off | on | Engine I latch |
| Chorus II | on/off | off | Engine II latch |
| Rate I | 0.2-2 Hz | 0.55 Hz | |
| Depth I | 0-100% | 25% | |
| Rate II | 0.2-2 Hz | 1.0 Hz | |
| Depth II | 0-100% | 55% | |
| Delay Center | 5-15 ms | 8 ms | Shared BBD tap centre |
| Decorrelation | 0-100% | 70% | L/R phase/delay/polarity difference - not panning |
| Drift | 0-100% | 40% | Slow BBD clock wander |
| Saturation | 0-100% | 15% | Tiny BBD-stage drive |
| Noise | 0-100% | 20% | BBD hiss floor |
| Mix | 0-100% | 50% | Dry/wet |
| Output Trim | -24..+24 dB | 0 dB | |

No Tone/HF-rolloff control - the BBD reconstruction filter's ~6-8kHz rolloff is a fixed DSP
characteristic, matching how the real hardware has no such knob.

## Building

See [BUILDING.md](BUILDING.md) for per-platform build requirements, commands, validation
(auval/pluginval on macOS, pluginval on Windows/Linux), and running the DSP test suite.

## Project layout

```
Source/
  PluginProcessor.*    Audio processor: parameter caching, signal chain, program management wiring
  PluginEditor.*        Editor shell, fixed-aspect-ratio scaling
  Parameters.h          APVTS parameter layout, IDs, and legacy-session migration seam
  DSP/
    ModulationEngine                          One chorus engine's rounded/asymmetric LFO (x2 instances)
    BBDDelayLine                              Shared delay buffer + input pre-filter + reconstruction filter
    StereoDecorrelationStage                  Real L/R decorrelation, not panning
    CharacterStage                            Drift (feeds tap position), Saturation, Noise floor
    OutputMixStage                            Dry/wet mix and output trim
    ProgramManager, FactoryPrograms           Factory (read-only) + user (read-write) program banks
  GUI/
    Chorus60LookAndFeel/Theme        Layout/colour constants (1400x632 reference canvas)
    Chorus60PanelBackground          Full static fascia bitmap - asset-based, same approach as Gatecrasher
    KnobFilmstripComponent           Bitmap sprite-sheet knobs (shared filmstrip assets with Gatecrasher)
    ModScope                         Live delay-modulation oscilloscope - the signature element
    EngineButtonComponent            The I/II/OFF hardware chorus buttons
    ProgramHeader                    Program LCD + name-entry flow (ported directly from Gatecrasher)
    WordmarkComponent                Live-drawn flat/clean wordmark (no baking needed, unlike Gatecrasher)
    Chorus60EditorContent            Assembles and positions all of the above
Tests/                   JUCE-UnitTest DSP unit tests (see BUILDING.md to run)
design/                  GUI spec, BBD technical notes, approved reference renders, bitmap/font assets
prompts/                 Numbered work-package prompts (gitignored, local-only)
```

## Status

See [CLAUDE.md](CLAUDE.md) for full architecture notes and current status, including the DSP
tuning and full factory-bank follow-ups tracked in `prompts/PROMPTS.md`.
