#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Deliberately thin, mirroring GatecrasherLookAndFeel: every control on this panel paints itself
// entirely in its own paint()/paintButton() override (KnobComponent, EngineButtonComponent,
// ModScope, ProgramHeader, etc.) rather than going through LookAndFeel::drawRotarySlider/drawButton.
// This class's job is just the handful of shared JUCE chrome that isn't owned by any one component:
// the fallback window background (visible only at the fixed-aspect-ratio letterboxing edges, if
// any) and the knob drag-value popup / shared TooltipWindow colours, kept consistent with the
// fascia palette.
class Chorus60LookAndFeel final : public juce::LookAndFeel_V4
{
public:
    Chorus60LookAndFeel();
};
