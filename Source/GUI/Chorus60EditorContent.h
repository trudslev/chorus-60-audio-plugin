#pragma once

#include "Chorus60Theme.h"
#include "Chorus60LookAndFeel.h"
#include "Chorus60PanelBackground.h"
#include "KnobFilmstripComponent.h"
#include "MonoStereoSwitch.h"
#include "ModScope.h"
#include "EngineButtonComponent.h"
#include "ProgramHeader.h"
#include "WordmarkComponent.h"
#include "PanelChrome.h"
#include "../PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

// Small live text readout beneath each knob. The name label above it (Barlow Condensed 600) is
// baked into the static panel background and never changes, so it's left alone; only the value
// line is redrawn live here, since CHORUS60-GUI-SPEC.md section 12's BRAND compliance notes call
// for "real ms/Hz/%/dB values under every knob" - Gatecrasher's own KnobFilmstripComponent has no
// permanent value readout at all (only a drag-time popup), which isn't accurate once a knob moves
// away from whatever value the reference screenshot happened to bake in. Positioned independently
// of KnobFilmstripComponent's own (unmodified, faithfully ported) bounds - see
// Chorus60Theme::Layout::knobLabelGap/knobNameRowH/knobValueRowH's comment for how the position was
// derived and cross-checked against the baked reference art.
class KnobValueLabel final : public juce::Component, private juce::Timer
{
public:
    explicit KnobValueLabel(juce::RangedAudioParameter& parameterToDisplay);

private:
    void paint(juce::Graphics&) override;
    void timerCallback() override;

    juce::RangedAudioParameter& parameter;
};

// The assembler: owns every knob/button/indicator plus their APVTS attachments and positions them
// from Chorus60Theme::Layout, mirroring GatecrasherEditorContent's role for Gatecrasher. Everything
// here draws in the fixed 1400x632 reference canvas; PluginEditor applies the single uniform scale
// transform on resize.
class Chorus60EditorContent final : public juce::Component,
                                    private juce::Timer
{
public:
    explicit Chorus60EditorContent(Chorus60AudioProcessor& processor);
    ~Chorus60EditorContent() override;

    void paintOverChildren(juce::Graphics&) override;

private:
    void timerCallback() override;

    // Re-points the four slots and the switch at another configuration's parameters. Called when
    // the latches change, which is the only thing that pages this box - there is no tab strip and
    // no separate navigation control anywhere on the panel (spec section 7a).
    void bindPage(const Chorus60Theme::Layout::EnginePage& page);

    // Advances the slot slew and the power-down fade by one frame.
    void advanceAnimation(float dtMs);

    Chorus60AudioProcessor& processorRef;
    Chorus60LookAndFeel lookAndFeel;

    // Draw order: the bare plate, then every silkscreened glyph on top of it, then the live
    // controls above that.
    Chorus60PanelBackground panelBackground;
    PanelChrome panelChrome;
    WordmarkComponent wordmark;

    // The five genuinely global knobs (CHARACTER + OUTPUT). Fixed parameters, unlike the slots.
    std::array<std::unique_ptr<KnobFilmstripComponent>, Chorus60Theme::Layout::knobs.size()> knobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,
               Chorus60Theme::Layout::knobs.size()> knobAttachments;
    std::array<std::unique_ptr<KnobValueLabel>, Chorus60Theme::Layout::knobs.size()> knobValueLabels;

    // --- The paged MOD ENGINE box -----------------------------------------------------------
    //
    // Four slots whose bound parameter changes with the page, plus the Mono/Stereo switch. The
    // components are created once and re-attached; recreating them per page would destroy the very
    // rotation the transition animates away from.
    static constexpr int numSlots = 4;
    std::array<std::unique_ptr<KnobFilmstripComponent>, numSlots> slotKnobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, numSlots> slotAttachments;
    std::array<std::unique_ptr<KnobValueLabel>, numSlots> slotValueLabels;

    MonoStereoSwitch monoSwitch;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> monoSwitchAttachment;

    // Which page is currently bound. Retained through the OFF state so that powering back up
    // returns to the page that was showing rather than resetting to I.
    const Chorus60Theme::Layout::EnginePage* currentPage = &Chorus60Theme::Layout::pageI;
    bool poweredDown = false;

    // Animation state. Slot travel is per-slot because each slot's target is its own parameter's
    // proportion; the Character/Output knobs share one 0->1 power factor instead, so the whole
    // lower panel winds down together rather than at nine independent rates (spec section 7a).
    std::array<float, numSlots> slotDisplay{};
    float powerFactor = 1.0f;
    float fadeFactor = 1.0f;
    juce::uint32 lastFrameMs = 0;

    // Button column (section 4): OFF has no APVTS binding of its own (see EngineButtonComponent's
    // own onClick handler), so only I/II get attachments.
    EngineButtonComponent buttonII, buttonI, buttonOff;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> engine1Attachment, engine2Attachment;

    // The paged MOD ENGINE box's single Ø8 title-row LED. One, not two: it reports "any engine
    // engaged" because the box now shows one configuration rather than one engine each.
    EngineLedIndicator groupLed;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> groupLedAttachment;

    ModScope modScope;
    ProgramHeader programHeader;

    // Single shared popup for every knob's setTooltip() text below - scoped to this component
    // (rather than nullptr/whole-desktop) so it only ever considers Chorus-60's own controls, same
    // pattern as Gatecrasher's own tooltipWindow.
    juce::TooltipWindow tooltipWindow{this};
};
