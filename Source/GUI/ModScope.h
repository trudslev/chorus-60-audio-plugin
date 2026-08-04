#pragma once

#include "../PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

// The delay-modulation oscilloscope - the product's centrepiece (CHORUS60-GUI-SPEC.md section 5,
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

    Chorus60AudioProcessor& processorRef;

    std::atomic<float>* engine1Raw = nullptr;
    std::atomic<float>* engine2Raw = nullptr;
    std::atomic<float>* depth1Raw = nullptr;
    std::atomic<float>* depth2Raw = nullptr;
    std::atomic<float>* delayCenterRaw = nullptr;
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
