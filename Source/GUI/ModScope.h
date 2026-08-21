#pragma once

#include "../PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

// The delay-modulation oscilloscope - the product's centrepiece (GUI-SPEC.md section 5,
// "11. What matters most" #1). Fully code-drawn, same visual grammar as Gatecrasher's GateScope
// (scrolling grid, glow+core red trace, grey input underlay) but reading Chorus-60's own signal.
//
// Polls processor.getModulationOffsetMs()/getDriftOffsetMs() on a ~60fps Timer and accumulates its
// own local scrolling-history ring buffer, since PluginProcessor deliberately doesn't maintain a
// shared one (see the comment on those getters in PluginProcessor.h). The trace is genuinely the
// real modulator signal: getModulationOffsetMs() already returns 0 from any disabled engine while
// getDriftOffsetMs() keeps moving regardless, so with both engines off the trace naturally settles
// onto the drift/noise floor instead of going dead flat, with no special-casing needed here.
//
// Also owns the caption row above the scope rect (section 5: "DELAY MODULATION" + the
// engine-state/depth/division readouts) - folded into this component rather than split out, since
// both need the same engine1/engine2/depth1/depth2 polling and there's no lamp in this row to
// justify a separate component (BRAND.md: the button column's LEDs are the panel's one live-state
// indicator system).
class ModScope final : public juce::Component, private juce::Timer
{
public:
    explicit ModScope(Chorus60AudioProcessor& processor);
    ~ModScope() override;

    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;

    /*  **The static half, cached.** This component measured 3.398 ms per paint — 22 % of this
        editor's attributable cost — and it redrew everything on a 60 Hz timer with nothing cached:
        the title, two status readouts, the well's gradient and its frame, none of which change per
        frame, alongside the scrolling grid and the trace, which do.

        Tracked text was the expensive part and the one that changed least: tracking is not native,
        so a tracked string is laid out glyph by glyph, and three of them were measured and drawn
        sixty times a second while the title never changed at all.

        **Keyed on device scale AND on the strings**, not on component size. The scale is what
        actually changes the pixels — a component whose bounds never move can still be asked to
        paint at 2x on one display and 1x on another — and the readouts are the one part of the
        static half that varies, so they belong in the key rather than outside the cache.

        **`setBufferedToImage` is the trap this avoids**, for the same reason
        `KnobComponent` says so: it re-renders on every repaint, which for a component that
        repaints at 60 Hz is the problem restated rather than solved. */
    void renderStaticLayer (float deviceScale, const juce::String& cacheKey);

    juce::Image staticLayer;
    float cachedDeviceScale = 0.0f;
    juce::String cachedStaticKey;

    Chorus60AudioProcessor& processorRef;

    // Depth and Delay Center are per-configuration now, so the scope cannot hold fixed pointers to
    // "the" depth or centre - it asks the processor which configuration is engaged and reads that
    // one. Only Noise stays a direct pointer, being genuinely global.
    std::atomic<float>* noiseRaw = nullptr;

    struct HistorySample
    {
        float traceMs = 0.0f;     // getModulationOffsetMs() + getDriftOffsetMs() at poll time
        float greyHalfPx = 0.0f;  // dry input-underlay half-height, precomputed at poll time
    };

    // Comfortably more than the scope's visible column count (2.0s window at 60fps = 120 frames)
    // so the ring buffer never runs out of look-back history.
    static constexpr int historySize = 180;
    std::array<HistorySample, historySize> history{};
    int writeIndex = 0;

    juce::Random random;
    float gridScrollPhase = 0.0f;
};
