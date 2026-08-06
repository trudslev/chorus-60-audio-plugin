#pragma once

#include "Chorus60Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

// The MONO / STEREO switch inside the paged MOD ENGINE box (spec section 7a).
//
// Drawn in code rather than blitted: section 7a states plainly that no sprite exists for this
// control, and offers "either cut a 2-frame filmstrip or implement it code-drawn from the values
// above". Code-drawn wins here because the thumb animates between positions on a spring curve - a
// 2-frame strip would have to snap, and the spec asks for 260ms of travel.
//
// Subclasses juce::Button purely for its click handling and ButtonAttachment compatibility, exactly
// as KnobFilmstripComponent subclasses Slider. Toggle state true = MONO (thumb down), false =
// STEREO (thumb up), matching the mono1/mono2/monoB parameters' own sense.
class MonoStereoSwitch final : public juce::Button,
                               private juce::Timer
{
public:
    MonoStereoSwitch();
    ~MonoStereoSwitch() override;

    void paintButton(juce::Graphics&, bool shouldDrawHighlighted, bool shouldDrawDown) override;

    // Drives the panel-wide power-down. The switch stays visible and keeps its position - nothing
    // is hidden in the OFF state - but stops responding, per section 7a's "all knobs and the switch
    // are non-interactive while powered down".
    void setPoweredDown(bool shouldBePoweredDown);

private:
    void timerCallback() override;
    void buttonStateChanged() override;

    // 0 = thumb at the top (STEREO), 1 = thumb at the bottom (MONO). Animated rather than derived
    // directly from the toggle state so the travel is visible.
    float thumbPosition = 0.0f;
    float thumbTarget = 0.0f;

    bool poweredDown = false;
    juce::uint32 lastFrameMs = 0;
};
