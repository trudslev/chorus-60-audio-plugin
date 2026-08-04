#include "WordmarkComponent.h"
#include "Chorus60Theme.h"

WordmarkComponent::WordmarkComponent()
{
    setInterceptsMouseClicks(false, false);
}

void WordmarkComponent::paint(juce::Graphics& g)
{
    using namespace Chorus60Theme;
    using namespace Chorus60Theme::Layout;

    static const juce::String text = "CHORUS-60";
    const auto font = wordmarkFont(34.0f);
    const float trackingPx = 34.0f * 0.02f;
    const juce::Rectangle<float> block(wordmarkX, wordmarkY, wordmarkW, wordmarkH);

    // Engraving shadow: "0 1px 0 rgba(0,0,0,.9)".
    drawTrackedText(g, text, font, trackingPx, block.translated(0.0f, 1.0f),
                     juce::Justification::centredLeft, juce::Colours::black.withAlpha(0.9f));

    // Ink bloom: "1px rgba(230,235,238,.5)" - the faint spread of screen-printed ink, the only
    // texture the wordmark gets (explicitly not distressed - section 8).
    const juce::Colour bloom(0xFFE6EBEE);
    for (const auto offset : { juce::Point<float>(-1.0f, 0.0f), juce::Point<float>(1.0f, 0.0f),
                                juce::Point<float>(0.0f, -1.0f), juce::Point<float>(0.0f, 1.0f) })
        drawTrackedText(g, text, font, trackingPx, block.translated(offset.x, offset.y),
                         juce::Justification::centredLeft, bloom.withAlpha(0.12f));

    // Crisp main text, flat/clean per section 8 (no spray, no speckle, no per-letter rotation).
    drawTrackedText(g, text, font, trackingPx, block, juce::Justification::centredLeft, Colour::engravedHeadingText);
}
