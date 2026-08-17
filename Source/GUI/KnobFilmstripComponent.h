#pragma once

#include "Chorus60Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

/**
    A knob, code-drawn into a cached static layer with only its pointer redrawn per frame. §3's two
    classes: Ø76 for the four MOD ENGINE slots, Ø56 for the five global controls.

    **THE NAME IS NOW WRONG AND IS KEPT FOR ONE MORE ROUND.** This rendered a 128-frame bitmap
    filmstrip until call 5 — "code-drawn, cached, no filmstrips" — retired the sheets. Renaming the
    class touches every construction site in the editor and would land in the same commit as the
    drawing change, which is the noise the `ProgramId` aliasing decision exists to avoid. It is
    renamed on its own, next to nothing else.

    Two sheets went with it: `knob_mod_84px_128f.png` and `knob_global_68px_128f.png`, whose caps
    were Ø84 and Ø68 — this casting's diameters *before* call 3 — so every frame had been resampled
    to a size it was not drawn at.

    **It draws the cap and the pointer and nothing else.** The tick ring, the numerals, the unit and
    the control label are `drawKnobScale`'s, drawn by the group's own printed layer so they dim with
    the box; the knob is a control and sits above them.

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

    void mouseDown (const juce::MouseEvent& e) override;   // Shift = 4x fine drag

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

    /** §7.2's OFF state: specular off and the cap's top light suppressed. It is part of the STATIC
        layer, so setting it invalidates the cache rather than being applied on top. */
    void setPoweredDown (bool);

    /** **Test seam: how many times the static layer has been rebuilt.** A cache that silently
        rebuilds every frame is indistinguishable from no cache by looking at the panel, and
        `setBufferedToImage` — the obvious alternative — does exactly that on a Slider, which
        repaints on every frame of a drag. Asserted in `KnobRenderTests`. */
    int staticLayerBuildCount() const noexcept { return staticLayerBuilds; }

private:
    Chorus60Theme::KnobFilmstripSize filmstripSize;
    float diameter;

    float displayProportionOverride = -1.0f;
    bool poweredDown = false;

    juce::Image staticLayer;
    int staticLayerBuilds = 0;
};
