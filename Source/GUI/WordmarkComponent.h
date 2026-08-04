#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// The "CHORUS-60" wordmark (CHORUS60-GUI-SPEC.md section 8). Unlike Gatecrasher's spray-stencil
// treatment (which needed a pre-baked distressed PNG), section 8 is explicit that this nameplate
// metaphor - a silkscreened synth-panel legend - is flat and clean, so it's drawn live from the
// embedded Librestile Extended Bold typeface: no per-letter rotation, no speckle, no halo, just an
// engraving shadow and a faint ink-bloom spread. Sits directly on top of the static panel
// background's own baked copy of the same wordmark (chorus60-panel-bypass@2x.png bakes one in at
// the same position, for identical reasons to Gatecrasher's own WordmarkComponent/panel-background
// overlap) - drawing live on top keeps it crisp rather than relying on the bitmap's own render.
class WordmarkComponent final : public juce::Component
{
public:
    WordmarkComponent();

    void paint(juce::Graphics&) override;
};
