#include "Chorus60PanelBackground.h"
#include "Chorus60Theme.h"

Chorus60PanelBackground::Chorus60PanelBackground()
{
    // Pure background - never intercepts mouse input, so every real control layered on top of it
    // still receives clicks normally.
    setInterceptsMouseClicks(false, false);
}

void Chorus60PanelBackground::paint(juce::Graphics& g)
{
    using namespace Chorus60Theme;

    // Drawn at the canvas size including the 1 px frame the export carries, so the plate's own
    // border is the panel's border and no coordinate needs adjusting for it. The editor's content
    // child is offset by that 1 px instead, which keeps every Layout constant a literal spec value.
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage(panelBackgroundImage(),
                juce::Rectangle<float>(0.0f, 0.0f, Layout::canvasWidth, Layout::canvasHeight),
                juce::RectanglePlacement::stretchToFit);
}
