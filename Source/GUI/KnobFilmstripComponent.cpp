#include "KnobFilmstripComponent.h"

KnobFilmstripComponent::KnobFilmstripComponent(Chorus60Theme::KnobFilmstripSize size, float diameterPx)
    : juce::Slider(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox),
      filmstripSize(size), diameter(diameterPx)
{
    setRotaryParameters(juce::degreesToRadians(Chorus60Theme::Layout::knobArcStartDegrees),
                         juce::degreesToRadians(Chorus60Theme::Layout::knobArcEndDegrees), true);

    // Section 8: "full range over 200 px, x0.25 with Shift". The Shift-key fine-drag scaling is
    // juce::Slider's own built-in behaviour for RotaryVerticalDrag, not something to reproduce here.
    setMouseDragSensitivity(200);
}

void KnobFilmstripComponent::paint(juce::Graphics& g)
{
    using namespace Chorus60Theme;

    const auto centre = getLocalBounds().toFloat().getCentre();

    const bool isMod = filmstripSize == KnobFilmstripSize::mod;
    const auto& sheet = isMod ? Layout::modSheet : Layout::globalSheet;
    const auto& strip = isMod ? knobModFilmstrip() : knobGlobalFilmstrip();

    // The frame is drawn into a box scaled so the CAP comes out at the section-8 diameter. For the
    // @1x strips capFraction is 1, so this is the diameter itself and matches the reference renders
    // exactly; for a padded sheet it grows the box so the shadow margin lands outside the cap rather
    // than eating into it. See FilmstripSheet's comment - conflating cap with frame pitch is the
    // mistake handoff section 4 calls out.
    const float boxSize = diameter / sheet.capFraction;
    const juce::Rectangle<int> box((int) std::round(centre.x - boxSize * 0.5f),
                                    (int) std::round(centre.y - boxSize * 0.5f),
                                    (int) std::round(boxSize), (int) std::round(boxSize));

    // sliderPos accounts for the parameter's own skew (Rate's 0.35) via the Slider's
    // NormalisableRange, set up by SliderAttachment from the bound RangedAudioParameter - so the
    // pointer's rotation always matches the parameter's true travel proportion, and therefore lands
    // on the plate's printed marks, which were placed from that same curve.
    //
    // The override, when set, is already a proportion of travel and needs no conversion: a page
    // slew works in the same units precisely so it is linear in *rotation* rather than in parameter
    // value, which for a skewed parameter are not the same motion.
    constexpr int lastFrame = Layout::knobFrameCount - 1;
    const int frame = juce::jlimit(0, lastFrame,
                                    (int) std::round(getDrawnProportion() * (float) lastFrame));

    const int srcX = (frame % sheet.columns) * sheet.framePx;
    const int srcY = (frame / sheet.columns) * sheet.framePx;

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage(strip, box.getX(), box.getY(), box.getWidth(), box.getHeight(),
                srcX, srcY, sheet.framePx, sheet.framePx);
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
