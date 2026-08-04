#pragma once

#include "../PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

// The hardware chorus section's three square buttons (CHORUS60-GUI-SPEC.md section 4) - no
// Gatecrasher equivalent exists, styled directly against design/assets/jn80-chorus-reference.jpeg
// per design/CLAUDE.md. One class covers all three roles via a constructor mode flag rather than
// three separate classes, since they share the same face/press-animation construction and differ
// only in colour, LED presence, and label text/latch-vs-derived state.
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
    // label's live brightness poll above (see .cpp).
    float pressOffsetPx = 0.0f;
};

// The MOD ENGINE I/II group panels' own small Ø8 title-row LED (section 7's group table: "Ø8 LED
// (engine I state) + title") - the same live indicator system as the button column's Ø15 LEDs,
// just echoed at the group panel. Implemented as a second, non-interactive juce::Button so it can
// be driven by its own ButtonAttachment bound to the same engine1/engine2 parameter - no separate
// timer needed, same instant-repaint-on-change mechanism as the main buttons' own LEDs.
class EngineLedIndicator final : public juce::Button
{
public:
    explicit EngineLedIndicator(juce::Rectangle<float> ledBoundsAbsolute);

private:
    void paintButton(juce::Graphics&, bool, bool) override;

    juce::Rectangle<float> ledBounds;
};
