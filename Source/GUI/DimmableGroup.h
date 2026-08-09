#pragma once

#include "Chorus60Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

/**
    One of the three control groups that dim in the OFF state — MOD ENGINE, CHARACTER, OUTPUT, each
    from its heading rule down (spec section 9).

    **This exists to make the dim a genuine multiply.** Section 9 and BRAND.md both insist the fade
    is `panel x 0.50`, not an alpha blend toward the background colour: multiplying scales every
    pixel toward black and preserves relative contrast, so the group reads as *darker*, while
    blending toward the panel field washes it toward one flat grey and reads as fog laid over the
    fascia. (In CSS the two coincide only because the backdrop happens to be near-black.)

    The trick is composition order. This component paints an **opaque black rect** first, outside
    the fade, then holds a child — `content()` — carrying its own crop of the plate plus every
    control in the group, with `setAlpha(k)` on it. JUCE flattens that whole subtree through
    `beginTransparencyLayer` and composites it once, so over black the result is exactly
    `k*src + (1-k)*0 = k*src`. At k = 1 the opaque plate crop hides the black entirely.

    Getting that ordering wrong quietly turns it back into the blend the spec forbids:

      - `setAlpha` on a child sitting directly over the already-painted plate blends toward the
        plate, not toward black.
      - Applying the factor per element (`g.setOpacity(k)` at each draw site) breaks wherever two
        elements overlap, because the lower one shows through the upper at `(1-k)`.

    Which regions dim, and where they start, were measured off `chorus60-page-off@2x.png` rather
    than inferred: the ratio against the bare plate is exactly 0.500 inside all three boxes and
    1.000 in their heading rows, the header band, the scope, the button column and the footer. The
    boundary sits 31 px below each box's top in all three cases.
*/
class DimmableGroup final : public juce::Component
{
public:
    /** @param regionInPanel  the dim rect in inside-border panel coordinates. */
    explicit DimmableGroup(juce::Rectangle<float> regionInPanel);

    void paint(juce::Graphics&) override;

    /** 1.0 = live, Layout::powerDownMultiply = bypassed. */
    void setDimFactor(float factor);

    /** Add controls here, not to the group itself: only this child is inside the faded subtree.
        Children keep drawing in absolute panel coordinates — the content is positioned at the
        negative of the region's origin so a component placed at its panel rect lands correctly. */
    juce::Component& content() noexcept { return contentLayer; }

private:
    /** Draws the group's own crop of the plate, so the plate dims with the controls over it — the
        printed scales and numerals are part of what powers down. */
    class ContentLayer final : public juce::Component
    {
    public:
        explicit ContentLayer(juce::Rectangle<float> regionInPanel) : region(regionInPanel) {}
        void paint(juce::Graphics&) override;

    private:
        juce::Rectangle<float> region;
    };

    juce::Rectangle<float> region;
    ContentLayer contentLayer;
    float dimFactor = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DimmableGroup)
};
