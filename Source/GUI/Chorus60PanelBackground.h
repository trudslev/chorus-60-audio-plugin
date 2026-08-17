#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// The panel plate: design/assets/chorus60-background-plate@3x.png, 4020 x 2436, drawn across the
// full 1340 x 812 canvas. Coordinates are the canvas's; the 1 px frame is part of the plate.
//
/*  **THE PLATE CARRIES SIX THINGS AND THIS COMMENT USED TO CLAIM IT CARRIED EVERYTHING.**

    Revision 2 inverted the plate's role, baking every static string on the fascia, and this file
    was the manifest for that: "redrawing one of those strings double-prints it, and baking a
    runtime one freezes it." Revision 4 inverts it back. Measured off the delivered @3x asset rather
    than read off a manifest, it holds:

      - the fascia gradient
      - the CHORUS badge and its foot bar
      - the scope well: frame, field and grid
      - the three box frames, their fields and their heading rules

    **and nothing else.** No header block, no LCD/IN/OUT well, no button face, no wordmark, no
    caption, no tick, no label. Its twelve box edges land on GUI-SPEC §1 to within a third of a
    canvas pixel, so §1 is safe to transcribe.

    **The failure mode inverted with it, and the new one is silent.** While the layer was baked the
    hazard was double-printing — visible, at a one-pixel offset. Now the same element fails by being
    ABSENT, and the panel merely looks emptier than the render. So the check this file owes is a
    completeness one: `../../CLAUDE.md`'s plate enumeration lists what stopped being carried, and it
    is derived from this asset rather than extended by hand, because the hand-written version missed
    thirteen rows by having no column for material.

    The four chorus60-page-*@2x renders are the PREVIOUS canvas and the previous treatment. They are
    not acceptance targets for this revision and measuring the new behaviour off them returns the
    old answer confidently. */
//
// The three DimmableGroup regions draw their OWN crop of this same plate rather than letting this
// component show through, because the OFF state has to multiply the plate along with the controls
// on top of it. Everything outside those three rects is this component's.
class Chorus60PanelBackground final : public juce::Component
{
public:
    Chorus60PanelBackground();

    void paint(juce::Graphics&) override;
};
