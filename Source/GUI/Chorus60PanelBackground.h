#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// The panel plate: design/assets/chorus60-background-plate@2x.png, 2564 x 1554, drawn across the
// full 1282 x 777 canvas (1280 x 775 of panel material plus the 1 px outer frame the export
// includes).
//
// Since revision 2 the plate is NOT bare. It bakes every static string on the fascia - the printed
// scales, every tick ring, numeral and unit, the wordmark, the group headings, the global control
// labels, the switch's printed STEREO/MONO positions and the PROGRAM/IN/OUT captions - along with
// the empty wells for the scope, the LCD and the two meters. design/CHORUS60-BUILD-HANDOFF.md
// section 1 is the manifest, and it cuts both ways: redrawing one of those strings double-prints
// it, and baking a runtime one freezes it.
//
// The three DimmableGroup regions draw their OWN crop of this same plate rather than letting this
// component show through, because the OFF state has to multiply the plate along with the controls
// on top of it. Everything outside those three rects is this component's.
//
// The four chorus60-page-*@2x renders are full composites - plate plus every runtime element - and
// are pixel-matching acceptance targets, never runtime assets and never a source to slice.
class Chorus60PanelBackground final : public juce::Component
{
public:
    Chorus60PanelBackground();

    void paint(juce::Graphics&) override;
};
