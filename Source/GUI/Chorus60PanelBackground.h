#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// The flat static background: design/assets/chorus60-panel-bypass@2x.png (both engine LEDs dark,
// OFF label bright, scope at drift/noise floor), drawn once across the full 1400x632 reference
// canvas. Chassis gradient/grain, the button column's base art (faces + blue stripes), group
// panels, footer stamp, and every static label are all baked into this single bitmap - see
// design/CLAUDE.md's "GUI approach" and chorus-60/CLAUDE.md's GUI section. Every other GUI
// component in this plugin is layered on top of this one, either painting over a specific region
// (ProgramHeader, EngineButtonComponent, the group-panel LEDs) or adding something the bitmap can't
// show at all (the knobs, the scope, the wordmark).
//
// The other two reference renders (chorus60-panel@2x.png, chorus60-panel-both-engines@2x.png) and
// gatecrasher-panel-reference.png/jn80-chorus-reference.jpeg are pixel-matching/authority targets
// for an implementer to check against, not runtime assets - deliberately excluded from
// juce_add_binary_data in CMakeLists.txt.
class Chorus60PanelBackground final : public juce::Component
{
public:
    Chorus60PanelBackground();

    void paint(juce::Graphics&) override;
};
