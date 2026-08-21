#include "KnobComponent.h"

KnobComponent::KnobComponent(Chorus60Theme::KnobFilmstripSize size, float diameterPx)
    : juce::Slider(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox),
      filmstripSize(size), diameter(diameterPx)
{
    setRotaryParameters(juce::degreesToRadians(Chorus60Theme::Layout::knobArcStartDegrees),
                         juce::degreesToRadians(Chorus60Theme::Layout::knobArcEndDegrees), true);

    // Section 8: "full range over 200 px, x0.25 with Shift". The Shift-key fine-drag scaling is
    // juce::Slider's own built-in behaviour for RotaryVerticalDrag, not something to reproduce here.
    setMouseDragSensitivity(Chorus60Theme::Layout::knobDragPixels);
}

void KnobComponent::paint(juce::Graphics& g)
{
    using namespace Chorus60Theme;

    /*  **CODE-DRAWN AND CACHED — call 5 retires the filmstrips.**

        This blitted one of 128 frames from a 168 x 21504 sheet. Two sheets shipped in BinaryData,
        the cap in each was Ø84 / Ø68 — this casting's previous diameters — and call 3 moved the
        classes to Ø76 / Ø56, so every frame was being resampled to a size it was not drawn at.

        **The split is static / pointer, and the line between them is what the value moves.** The
        cap, its rim and its specular do not: §3 fixes the specular to the panel rather than to the
        knob, so it belongs in the layer that is drawn once. Only the pointer rotates.

        **The cache is keyed on the DEVICE scale, not on the component size.** A Retina panel and a
        scaled editor both change how many physical pixels the cap covers while its logical bounds
        stay put, so a cache keyed on `getWidth()` would serve a blurry layer after a move between
        displays. `staticLayerBuilds` counts rebuilds so a test can assert the cache is a cache —
        `setBufferedToImage` is the trap this avoids, since it re-renders on every repaint and a
        knob repaints on every frame of a drag.  */
    const auto bounds = getLocalBounds().toFloat();
    const auto centre = bounds.getCentre();

    const float deviceScale = g.getInternalContext().getPhysicalPixelScaleFactor();
    const int wantedW = juce::roundToInt (bounds.getWidth() * deviceScale);
    const int wantedH = juce::roundToInt (bounds.getHeight() * deviceScale);

    if (staticLayer.isNull() || staticLayer.getWidth() != wantedW
        || staticLayer.getHeight() != wantedH)
    {
        staticLayer = juce::Image (juce::Image::ARGB, juce::jmax (1, wantedW),
                                    juce::jmax (1, wantedH), true);
        ++staticLayerBuilds;

        juce::Graphics ig { staticLayer };
        ig.addTransform (juce::AffineTransform::scale (deviceScale));
        paintKnobCap (ig, bounds.withZeroOrigin(), diameter, poweredDown);
    }

    g.drawImageTransformed (staticLayer, juce::AffineTransform::scale (1.0f / deviceScale));

    /*  The pointer. Its angle comes from the Slider's own `valueToProportionOfLength`, which carries
        the parameter's skew (RATE's 0.35) through `SliderAttachment` — so the pointer lands on the
        printed marks, which `drawKnobScale` places from that same range rather than from a stored
        fraction.

        The override, when set, is already a proportion of TRAVEL and needs no conversion: a page
        slew works in those units precisely so it is linear in rotation rather than in parameter
        value, which for a skewed parameter are not the same motion. */
    paintKnobPointer (g, centre, diameter, getDrawnProportion());
}

void KnobComponent::setDisplayProportion(float proportion) noexcept
{
    const float clamped = juce::jlimit(0.0f, 1.0f, proportion);
    if (std::abs(clamped - displayProportionOverride) < 1.0e-5f)
        return;
    displayProportionOverride = clamped;
    repaint();
}

void KnobComponent::setPoweredDown(bool nowPoweredDown)
{
    if (poweredDown == nowPoweredDown)
        return;
    poweredDown = nowPoweredDown;

    // The specular is in the static layer, so this invalidates it rather than drawing over it.
    staticLayer = {};
    repaint();
}

void KnobComponent::clearDisplayProportion() noexcept
{
    if (displayProportionOverride < 0.0f)
        return;
    displayProportionOverride = -1.0f;
    repaint();
}

float KnobComponent::getDrawnProportion()
{
    if (displayProportionOverride >= 0.0f)
        return displayProportionOverride;
    return (float) valueToProportionOfLength(getValue());
}

void KnobComponent::mouseDown(const juce::MouseEvent& e)
{
    // Sensitivity has to be settled BEFORE Slider::mouseDown records its drag anchor: JUCE measures
    // from that anchor and scales by the current sensitivity, so changing it mid-drag rescales the
    // distance already travelled and the value jumps.
    setMouseDragSensitivity(e.mods.isShiftDown() ? Chorus60Theme::Layout::knobFineDragPixels
                                                 : Chorus60Theme::Layout::knobDragPixels);

    juce::Slider::mouseDown(e);
}
