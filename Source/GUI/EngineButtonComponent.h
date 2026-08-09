#pragma once

#include "../PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

// The hardware chorus section's three square buttons (spec section 4), 132 x 132 at x 26. One class
// covers all three roles via a constructor mode flag rather than three separate classes, since they
// share the same sprite/press-animation construction and differ only in which sprite, whether a lamp
// sits beside them, and label text/latch-vs-derived state.
//
// Faces are SPRITES and are state-independent: on the JN-80 the buttons are plain moulded plastic
// that never illuminates, so there is no lit/unlit pair. The lamp beside each one carries the state.
// The letters are still drawn in code, because their colour follows engagement.
//
// Subclasses juce::Button (not plain Component) so the engineI/engineII instances can bind to the
// engine1/engine2 bool APVTS parameters via a standard
// juce::AudioProcessorValueTreeState::ButtonAttachment, owned externally by Chorus60EditorContent -
// Button::setToggleState (called by the attachment whenever the parameter changes, from any
// source: click, automation, program load) repaints synchronously, which is what gives the LED its
// required "no fade" instant flip without this class needing its own polling timer for that part.
enum class EngineButtonRole { engineI, engineII, off };

class EngineButtonComponent final : public juce::Button, private juce::Timer
{
public:
    EngineButtonComponent(Chorus60AudioProcessor& processor, EngineButtonRole role);
    ~EngineButtonComponent() override;

    bool hitTest(int x, int y) override;

private:
    void paintButton(juce::Graphics&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void timerCallback() override;

    Chorus60AudioProcessor& processorRef;
    EngineButtonRole role;

    // OFF-mode only: derived (not owned) label-brightness state - bright when neither engine is
    // engaged. Read directly off the APVTS raw parameter pointers rather than via ButtonAttachment,
    // since OFF itself is not a latch and has no parameter of its own to bind to.
    std::atomic<float>* engine1Raw = nullptr;
    std::atomic<float>* engine2Raw = nullptr;

    // 3px-down/110ms press animation (section 4), eased by timerCallback toward
    // Button::isDown()'s current state - continuous even when idle so it also drives the OFF
    // label's live brightness poll above (see .cpp). Named pressOffset, not pressOffsetPx, so it
    // doesn't shadow Layout::pressOffsetPx, which is the constant it eases toward.
    float pressOffset = 0.0f;
};

// The MOD ENGINE box's own Ø8 heading-row indicator, lit whenever any engine is engaged. Measured
// at (308.5, 268.5) off chorus60-page-i@2x.png, where it is lit, against page-off, where it is dark.
// Implemented as a second, non-interactive juce::Button so it can be driven by its own
// ButtonAttachment - no separate timer, same instant-repaint-on-change mechanism as the buttons.
class EngineLedIndicator final : public juce::Button
{
public:
    explicit EngineLedIndicator(juce::Rectangle<float> ledBoundsAbsolute);

private:
    void paintButton(juce::Graphics&, bool, bool) override;

    juce::Rectangle<float> ledBounds;
};
