#pragma once

#include "Chorus60Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

/**
    A knob rendered from a 128-frame bitmap filmstrip rather than code-drawn, in the two diameters
    revision 2 enlarged the panel to: Ø84 for the four Mod Engine slots, Ø68 for the five global
    controls (spec section 6). Frame 0 is −135°, frame 127 is +135°, linear between.

    **It draws the knob and nothing else.** The tick ring this used to paint is gone: revision 2
    bakes every tick into the plate at its *labelled* value (section 7), which on the skewed Rate
    knob is not evenly spaced, so a drawn ring at even angles would lay wrong marks over right ones.
    The printed scale replaces the knurl ring too.

    The sheet's geometry arrives as data (`Chorus60Theme::FilmstripSheet`) rather than being assumed,
    because the @1x strips shipping today are an interim: the @2x sheets need Ø168/Ø136, which is
    above the Ø128 masters they were made from, so they are upsampled placeholders until someone
    with the original knob source re-renders them. Those will be 8-column row-major grids rather than
    vertical strips — a table edit here, not a code change.

    Subclasses juce::Slider purely for its drag-to-value mapping and SliderAttachment compatibility;
    paint() fully replaces the default look. Drag is vertical over 200 px of travel with a
    double-click reset (section 8), and because a rotary Slider works in normalised proportion the
    Mod Engine knobs automatically drag in *sweep fraction* rather than in Hz — which is what makes
    the skewed Rate knob feel even under the hand.
*/
class KnobFilmstripComponent final : public juce::Slider
{
public:
    KnobFilmstripComponent(Chorus60Theme::KnobFilmstripSize size, float diameterPx);

    void paint(juce::Graphics&) override;

    /** Draw at a rotation other than the parameter's own, so a page change can slew each pointer to
        its new value instead of snapping (section 10). Passing a negative value returns the knob to
        following its parameter directly.

        This is NOT used for the OFF state. Revision 2 deleted the wind-to-zero: real hardware
        doesn't move its knobs when a lamp goes out, and rotating pointers to zero depicted values
        that weren't current — which is the whole reason the panel used to have to print
        "SETTINGS RETAINED" to correct the impression. */
    void setDisplayProportion(float proportion) noexcept;
    void clearDisplayProportion() noexcept;

    /** The proportion currently drawn, whether from the override or the parameter. The editor's
        slew needs it as a starting point. */
    float getDrawnProportion();

private:
    Chorus60Theme::KnobFilmstripSize filmstripSize;
    float diameter;

    float displayProportionOverride = -1.0f;
};
