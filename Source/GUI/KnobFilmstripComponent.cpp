#include "KnobFilmstripComponent.h"

KnobFilmstripComponent::KnobFilmstripComponent(Chorus60Theme::KnobFilmstripSize size, float diameterPx,
                                                 float tickSpacingDegreesIn)
    : juce::Slider(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox),
      filmstripSize(size), diameter(diameterPx), tickSpacingDegrees(tickSpacingDegreesIn)
{
    setRotaryParameters(juce::degreesToRadians(Chorus60Theme::Layout::knobArcStartDegrees),
                         juce::degreesToRadians(Chorus60Theme::Layout::knobArcEndDegrees), true);

    // Section 7: "full range over 180px of travel, Shift = 0.25x fine" - the shift-key fine-drag
    // scaling itself is juce::Slider's own built-in behaviour for RotaryVerticalDrag, not something
    // this component needs to reproduce by hand.
    setMouseDragSensitivity(180);
}

void KnobFilmstripComponent::paint(juce::Graphics& g)
{
    using namespace Chorus60Theme;

    const auto bounds = getLocalBounds().toFloat();
    const auto centre = bounds.getCentre();
    const float radius = diameter * 0.5f;

    // Tick ring - not part of the filmstrip (it doesn't rotate with the knob), drawn first so the
    // filmstrip frame's own baked cast-shadow bleed naturally overlaps its inner edge.
    const int tickCount = tickCountForSpacing(tickSpacingDegrees);
    for (int i = 0; i < tickCount; ++i)
    {
        const float angle = Layout::knobArcStartDegrees
            + (float) i / (float) (tickCount - 1)
              * (Layout::knobArcEndDegrees - Layout::knobArcStartDegrees);
        const auto inner = pointOnCircle(centre, radius + Layout::tickInnerOffset, angle);
        const auto outer = pointOnCircle(centre, radius + Layout::tickOuterOffset, angle);
        g.setColour(Colour::tickMark.withAlpha(0.55f));
        g.drawLine({inner, outer}, 1.0f);
    }

    // Filmstrip frame - drawn into the knob's full bounding box (diameter + ~7% bleed), not just
    // the knob circle, since the cast shadow is baked in and bleeds outside the circle (section 7).
    const float boxSize = diameter * Layout::knobBoundingBoxBleed;
    const juce::Rectangle<int> box((int) std::round(centre.x - boxSize * 0.5f),
                                    (int) std::round(centre.y - boxSize * 0.5f),
                                    (int) std::round(boxSize), (int) std::round(boxSize));

    const auto& strip = filmstripSize == KnobFilmstripSize::large ? knobLargeFilmstrip() : knobSmallFilmstrip();

    // sliderPos accounts for the parameter's own skew (e.g. Rate's log skew) via the Slider's
    // NormalisableRange, set up by SliderAttachment from the bound RangedAudioParameter - so the
    // knob's physical rotation always matches the parameter's true travel proportion.
    //
    // The override, when set, is already a proportion of travel, so it needs no conversion: the
    // animator works in the same units precisely so a slew is linear in *rotation* rather than in
    // parameter value, which for a skewed parameter are not the same motion.
    const float sliderPos = getDrawnProportion();
    const int frame = juce::jlimit(0, 127, (int) std::round(sliderPos * 127.0f));

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.setOpacity(dimFactor);
    g.drawImage(strip, box.getX(), box.getY(), box.getWidth(), box.getHeight(),
                0, frame * 128, 128, 128);
    g.setOpacity(1.0f);
}

void KnobFilmstripComponent::setDisplayProportion(float proportion) noexcept
{
    const float clamped = juce::jlimit(0.0f, 1.0f, proportion);
    if (std::abs(clamped - displayProportionOverride) < 1.0e-5f)
        return;
    displayProportionOverride = clamped;
    repaint();
}

void KnobFilmstripComponent::clearDisplayProportion() noexcept
{
    if (displayProportionOverride < 0.0f)
        return;
    displayProportionOverride = -1.0f;
    repaint();
}

float KnobFilmstripComponent::getDrawnProportion()
{
    if (displayProportionOverride >= 0.0f)
        return displayProportionOverride;
    return (float) valueToProportionOfLength(getValue());
}

void KnobFilmstripComponent::setDimFactor(float factor) noexcept
{
    const float clamped = juce::jlimit(0.0f, 1.0f, factor);
    if (std::abs(clamped - dimFactor) < 1.0e-4f)
        return;
    dimFactor = clamped;
    repaint();
}
