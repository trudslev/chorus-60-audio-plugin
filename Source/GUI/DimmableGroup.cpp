#include "DimmableGroup.h"

DimmableGroup::DimmableGroup(juce::Rectangle<float> regionInPanel)
    : region(regionInPanel), contentLayer(regionInPanel)
{
    // The group itself is a plain container; the controls live one level down so exactly one
    // subtree carries the alpha. It must not swallow clicks itself - but the SECOND argument has to
    // stay true, or it blocks its children as well and every knob inside the box goes dead.
    setInterceptsMouseClicks(false, true);

    setBounds(region.getSmallestIntegerContainer());

    // Children go on drawing in absolute panel coordinates: shifting the content layer by the
    // region's negative origin means a control positioned at its panel rect lands where the plate
    // expects it, with no per-component offset arithmetic to get wrong.
    contentLayer.setBounds(-getX(), -getY(),
                            (int) Chorus60Theme::Layout::contentWidth,
                            (int) Chorus60Theme::Layout::contentHeight);
    contentLayer.setInterceptsMouseClicks(false, true);
    addAndMakeVisible(contentLayer);
}

void DimmableGroup::paint(juce::Graphics& g)
{
    // Opaque black, painted OUTSIDE the fade. This is the backdrop that turns the content layer's
    // alpha into a multiply - see the class comment. Without it the fade blends toward whatever the
    // plate already put here, which is the treatment section 9 explicitly rejects.
    if (dimFactor < 1.0f)
        g.fillAll(juce::Colours::black);
}

void DimmableGroup::setDimFactor(float factor)
{
    const float clamped = juce::jlimit(0.0f, 1.0f, factor);
    if (std::abs(clamped - dimFactor) < 1.0e-4f)
        return;

    dimFactor = clamped;

    // setAlpha on the CONTENT, not on this component: JUCE flattens a sub-1.0-alpha component and
    // its children into one transparency layer and composites that once, which is exactly the
    // "flatten then multiply" order needed. Alpha on the group itself would fade the black backdrop
    // along with it and defeat the whole arrangement.
    contentLayer.setAlpha(dimFactor);
    repaint();
}

void DimmableGroup::ContentLayer::paint(juce::Graphics& g)
{
    using namespace Chorus60Theme;

    // This layer's own crop of the plate. The printed scales, numerals, units and group headings
    // inside the box are baked, and section 9 dims them along with the controls - so the plate has
    // to be inside the faded subtree rather than showing through from behind it.
    const auto& plate = panelBackgroundImage();
    const float scale = (float) plate.getWidth() / Layout::canvasWidth;

    // Source rect in plate pixels. +borderInset converts inside-border panel coordinates to the
    // exported bitmap's own, which includes the 1 px frame.
    const juce::Rectangle<int> src(
        (int) std::round((region.getX() + Layout::borderInset) * scale),
        (int) std::round((region.getY() + Layout::borderInset) * scale),
        (int) std::round(region.getWidth() * scale),
        (int) std::round(region.getHeight() * scale));

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage(plate,
                 (int) std::round(region.getX()), (int) std::round(region.getY()),
                 (int) std::round(region.getWidth()), (int) std::round(region.getHeight()),
                 src.getX(), src.getY(), src.getWidth(), src.getHeight());
}
