#pragma once

#include "Chorus60Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

// A knob rendered from a 128-frame bitmap filmstrip (design/assets/knob_large_128px_128f.png /
// knob_small_128px_128f.png - literally the same two files Gatecrasher ships) rather than
// code-drawn. Ported directly from Gatecrasher's own KnobFilmstripComponent per design/CLAUDE.md's
// explicit instruction, re-pointed at Chorus60Theme and CHORUS60-GUI-SPEC.md section 7's own tick
// geometry (r+3..r+9, 15/20deg spacing) and interaction contract (RotaryVerticalDrag, 180px full
// travel, double-click reset - section 7's "Interaction" paragraph, more explicit than Gatecrasher's
// own knob wiring). There's no algorithm-selector knob on this plugin (unlike Gatecrasher's
// Algorithm), so that special-case is dropped entirely.
//
// Subclasses juce::Slider purely for its click/drag-to-value mapping and SliderAttachment
// compatibility; paint() fully replaces the default look. The tick ring is drawn here in code
// (section 7: "not part of the filmstrip - it does not rotate"), underneath the filmstrip frame so
// the frame's own baked cast-shadow bleed can naturally overlap the ring's inner edge.
class KnobFilmstripComponent final : public juce::Slider
{
public:
    KnobFilmstripComponent(Chorus60Theme::KnobFilmstripSize size, float diameterPx, float tickSpacingDegrees);

    void paint(juce::Graphics&) override;

    // Draw at a rotation other than the parameter's own, so the knob can slew between values
    // instead of snapping (spec section 7a) and can wind down to minimum while the panel is
    // powered off *without* touching the parameter - OFF reads "SETTINGS RETAINED", so the values
    // must survive untouched while the panel visibly winds down.
    //
    // Passing a negative value returns the knob to following its parameter directly.
    void setDisplayProportion(float proportion) noexcept;
    void clearDisplayProportion() noexcept;

    // The proportion currently drawn, whether that comes from the override or the parameter. The
    // editor's animator needs this as the starting point of a slew.
    float getDrawnProportion();

    // Dimming factor for the power-down fade, 1 = normal.
    void setDimFactor(float factor) noexcept;

private:
    Chorus60Theme::KnobFilmstripSize filmstripSize;
    float diameter;
    float tickSpacingDegrees;

    float displayProportionOverride = -1.0f;
    float dimFactor = 1.0f;
};
