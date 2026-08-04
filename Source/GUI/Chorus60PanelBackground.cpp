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

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage(panelBackgroundImage(),
                juce::Rectangle<float>(0.0f, 0.0f, Layout::canvasWidth, Layout::canvasHeight),
                juce::RectanglePlacement::stretchToFit);
}
