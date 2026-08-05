#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Every fixed glyph silkscreened onto the fascia: the model lines beside the wordmark, the blue
// stripe's CHORUS caption, each group panel's title, each knob's name label, the IN / OUT captions,
// and the footer status line.
//
// This exists because the background is `assets/chorus60-background-plate@2x.png` - the bare fascia,
// with wells, stripes, group boxes and heading rules but no controls and no text. Per the spec's
// preamble, "every glyph on the panel is drawn by the host, not baked in".
//
// It replaced a fully dressed render (chorus60-panel-bypass@2x.png) that carried baked copies of the
// knobs, labels and value readouts. Compositing live elements over that meant every one of them sat
// on a frozen copy of itself - knob filmstrips over baked knobs whose pointers showed through at the
// wrong angle, live readouts over stale numbers. Gatecrasher hit the same wall and needed a bare
// chassis to get out of it; this is the same fix, made before the symptoms had to be chased.
//
// Type sizes come from the spec as CSS px and are converted through
// Chorus60Theme::labelFontHeightForCssPx - see that function for why the two aren't the same number.
//
// Purely static: painted once, no timer, never intercepts mouse input. Anything state-dependent
// lives elsewhere - the knob value labels, the scope's own caption row and annotations, the engine
// buttons' legends, and ProgramHeader's LCD text.
class PanelChrome final : public juce::Component
{
public:
    PanelChrome();

    void paint(juce::Graphics&) override;

private:
    void paintHeader(juce::Graphics&);
    void paintGroupTitles(juce::Graphics&);
    void paintKnobLabels(juce::Graphics&);
    void paintFooter(juce::Graphics&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PanelChrome)
};
