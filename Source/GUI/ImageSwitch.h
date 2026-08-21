#pragma once

#include "Chorus60Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

/**
    The IMAGE switch inside the paged MOD ENGINE box (spec section 7.2).

    **The control is IMAGE; MONO and STEREO are its positions**, printed beside the thumb on the
    plate at the exact y each position puts the thumb — the same rule as the knob ticks, that the
    print sits at the position it names. The parameters are named to match (`image1`/`image2`/
    `imageB`), while their value strings stay `MONO` / `STEREO` so the host reports what the fascia
    prints.

    Built from two sprites: a 34 x 68 empty track and a 26 px thumb drawn over it. Revision 2 asked
    the designers for exactly that split, because the composite `switch-mono` / `switch-stereo`
    sprites bake the thumb at each end and could only be crossfaded — the spec wants 34 px of real
    travel on a spring curve.

    Subclasses juce::Button purely for its click handling and ButtonAttachment compatibility, exactly
    as KnobComponent subclasses Slider. Toggle state true = MONO (thumb down), false =
    STEREO (thumb up), matching the image parameters' own sense.
*/
class ImageSwitch final : public juce::Button,
                          private juce::Timer
{
public:
    ImageSwitch();
    ~ImageSwitch() override;

    void paintButton(juce::Graphics&, bool shouldDrawHighlighted, bool shouldDrawDown) override;

    /** Stops the switch responding while the panel is bypassed. It stays visible and keeps its
        position — nothing is hidden and nothing moves in the OFF state; the group it sits in is
        simply multiplied down. */
    void setPoweredDown(bool shouldBePoweredDown);

private:
    void timerCallback() override;
    void buttonStateChanged() override;

    // 0 = thumb at the top (STEREO), 1 = thumb at the bottom (MONO). Animated rather than derived
    // straight from the toggle state so the travel is visible.
    float thumbPosition = 0.0f;
    float thumbTarget = 0.0f;

    bool poweredDown = false;
    juce::uint32 lastFrameMs = 0;
};
