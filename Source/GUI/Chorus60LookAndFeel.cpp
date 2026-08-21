#include "Chorus60LookAndFeel.h"
#include "Chorus60Theme.h"

Chorus60LookAndFeel::Chorus60LookAndFeel()
{
    // Near-black fallback - only ever visible for a frame before Chorus60PanelBackground paints
    // over it (section 1's chassis gradient sits around #141618 -> #0A0C0D).
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFF0E1012));

    // Knob drag-value popup (KnobComponent's setPopupDisplayEnabled) and the shared
    // TooltipWindow - styled like an LED window so they read as part of the same instrument.
    setColour(juce::BubbleComponent::backgroundColourId, Chorus60Theme::Colour::ledWindowBg);
    setColour(juce::BubbleComponent::outlineColourId, Chorus60Theme::Colour::ledWindowBorder);
    setColour(juce::TooltipWindow::backgroundColourId, Chorus60Theme::Colour::ledWindowBg);
    setColour(juce::TooltipWindow::outlineColourId, Chorus60Theme::Colour::ledWindowBorder);
    setColour(juce::TooltipWindow::textColourId, Chorus60Theme::Colour::ledWindowText);
}
