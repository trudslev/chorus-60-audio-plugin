#pragma once

#include "Chorus60Theme.h"
#include "Chorus60LookAndFeel.h"
#include "Chorus60PanelBackground.h"
#include "KnobFilmstripComponent.h"
#include "ModScope.h"
#include "EngineButtonComponent.h"
#include "ProgramHeader.h"
#include "WordmarkComponent.h"
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
class Chorus60EditorContent final : public juce::Component
{
public:
    explicit Chorus60EditorContent(Chorus60AudioProcessor& processor);
    ~Chorus60EditorContent() override;

private:
    Chorus60AudioProcessor& processorRef;
    Chorus60LookAndFeel lookAndFeel;

    Chorus60PanelBackground panelBackground;
    WordmarkComponent wordmark;

    std::array<std::unique_ptr<KnobFilmstripComponent>, Chorus60Theme::Layout::knobs.size()> knobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,
               Chorus60Theme::Layout::knobs.size()> knobAttachments;
    std::array<std::unique_ptr<KnobValueLabel>, Chorus60Theme::Layout::knobs.size()> knobValueLabels;

    // Button column (section 4): OFF has no APVTS binding of its own (see EngineButtonComponent's
    // own onClick handler), so only I/II get attachments.
    EngineButtonComponent buttonII, buttonI, buttonOff;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> engine1Attachment, engine2Attachment;

    // MOD ENGINE I/II group panels' own Ø8 LED echo (section 7's group table) - bound to the same
    // two parameters via their own attachments.
    EngineLedIndicator groupLedI, groupLedII;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> groupLedIAttachment, groupLedIIAttachment;

    ModScope modScope;
    ProgramHeader programHeader;

    // Single shared popup for every knob's setTooltip() text below - scoped to this component
    // (rather than nullptr/whole-desktop) so it only ever considers Chorus-60's own controls, same
    // pattern as Gatecrasher's own tooltipWindow.
    juce::TooltipWindow tooltipWindow{this};
};
