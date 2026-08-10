#pragma once

#include "Chorus60Theme.h"
#include "Chorus60LookAndFeel.h"
#include "DimmableGroup.h"
#include "KnobFilmstripComponent.h"
#include "ImageSwitch.h"
#include "ModScope.h"
#include "EngineButtonComponent.h"
#include "ProgramHeader.h"
#include "../PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

/**
    The five page-suffixed labels under the MOD ENGINE knob row - `RATE I+II`, `DEPTH I+II`,
    `DELAY CENTER I+II`, `DECORRELATION I+II` and `IMAGE I+II`.

    Drawn rather than baked because the suffix follows the page, which is the one thing about that
    row that changes. Everything else printed in the box - the scales, the numerals, the units - is
    silkscreen. Lives inside the MOD ENGINE group's dimmable subtree, because it sits below the
    heading rule and so dims with the controls it names.
*/
class ModSlotLabels final : public juce::Component
{
public:
    ModSlotLabels();
    void setPage(const Chorus60Theme::Layout::EnginePage&);

private:
    void paint(juce::Graphics&) override;

    const Chorus60Theme::Layout::EnginePage* page = &Chorus60Theme::Layout::pageI;
};

/**
    The assembler: owns every control plus its APVTS attachment, positions them from
    Chorus60Theme::Layout, and drives the page switch and the OFF state.

    Everything draws in inside-border panel coordinates on a fixed 1280 x 775 content area, which
    PluginEditor places at (1, 1) inside the 1282 x 777 plate and scales as a unit on resize.

    **There is no static-text layer.** Revision 2's plate bakes every string whose characters and
    colour never change, so `PanelChrome` and `WordmarkComponent` are gone rather than adapted. Only
    five strings are drawn here, all of them because they carry live state: the MOD ENGINE heading,
    its status note, the four page-suffixed slot labels plus the IMAGE label, and the footer.

    The three group boxes are `DimmableGroup`s, not plain containers. Section 9's OFF state
    multiplies each box below its heading rule by 0.50, and a multiply has requirements about
    composition order that a container satisfies and a bare `setAlpha` does not - see
    DimmableGroup's class comment. Controls therefore go into `group.content()`, never onto this
    component directly.
*/
class Chorus60EditorContent final : public juce::Component,
                                    private juce::Timer
{
public:
    explicit Chorus60EditorContent(Chorus60AudioProcessor& processor);
    ~Chorus60EditorContent() override;

    void paintOverChildren(juce::Graphics&) override;

private:
    void timerCallback() override;

    // Re-points the four slots and the switch at another configuration's parameters. Called when the
    // latches change, which is the only thing that pages this box - the physical I/II/OFF buttons
    // are the pager, and there is no tab strip or page arrow anywhere on the panel (section 10).
    void bindPage(const Chorus60Theme::Layout::EnginePage& page);

    // Routes a control's own drag to the LCD's parameter readout (section 5). Guarded on the
    // control's drag state by the caller, never on the attachment's callback - see
    // ProgramHeader::showParameter.
    void attachReadout(juce::Slider& knob, const juce::String& paramID);

    // Advances the page slew and the power-down fade by one frame.
    void advanceAnimation(float dtMs);

    Chorus60AudioProcessor& processorRef;
    Chorus60LookAndFeel lookAndFeel;

    DimmableGroup modEngineGroup, characterGroup, outputGroup;
    ModSlotLabels slotLabels;

    // The five genuinely global knobs (CHARACTER + OUTPUT). Fixed parameters, unlike the slots, and
    // no value labels beneath them - revision 2 removed the standing readouts entirely; the LCD is
    // the only numeric display on the panel.
    std::array<std::unique_ptr<KnobFilmstripComponent>, Chorus60Theme::Layout::knobs.size()> knobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,
               Chorus60Theme::Layout::knobs.size()> knobAttachments;

    // --- The paged MOD ENGINE box -------------------------------------------------------------
    //
    // Four slots whose bound parameter changes with the page, plus the IMAGE switch. The components
    // are created once and re-attached; recreating them per page would destroy the very rotation the
    // transition animates away from.
    static constexpr int numSlots = 4;
    std::array<std::unique_ptr<KnobFilmstripComponent>, numSlots> slotKnobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, numSlots> slotAttachments;

    ImageSwitch imageSwitch;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> imageSwitchAttachment;

    // Which page is currently bound. Retained through the OFF state so powering back up returns to
    // the page that was showing rather than resetting to I.
    const Chorus60Theme::Layout::EnginePage* currentPage = &Chorus60Theme::Layout::pageI;
    bool poweredDown = false;

    // Page-change slew, per slot because each slot's target is its own parameter's proportion. There
    // is no power-down rotation any more: section 9 deleted the wind-to-zero, so a bypassed panel
    // holds every pointer exactly where it is.
    std::array<float, numSlots> slotDisplay{};
    float dimFactor = 1.0f;
    juce::uint32 lastFrameMs = 0;

    // Button column: OFF has no APVTS binding of its own (see EngineButtonComponent's onClick), so
    // only I/II get attachments.
    EngineButtonComponent buttonII, buttonI, buttonOff;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> engine1Attachment, engine2Attachment;

    // The MOD ENGINE box's Ø8 heading-row indicator. It sits above the heading rule, so it belongs
    // to this component rather than to the dimmable group - it goes dark in bypass, it doesn't dim.
    EngineLedIndicator groupLed;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> groupLedAttachment;

    ModScope modScope;
    ProgramHeader programHeader;

    /** Paints nothing and claims no clicks of its own; it exists so the Program list has a parent
        area to be laid out in. Its bounds are what stop the list moving or overflowing the panel -
        see the constructor, and ../../CLAUDE.md's "The Program dropdown". */
    juce::Component menuHost;

    // Single shared popup for every knob's setTooltip() text - scoped to this component (rather than
    // nullptr/whole-desktop) so it only ever considers Chorus-60's own controls. Static descriptive
    // tooltips stay; the drag-time value popup is gone with the standing readouts, because the LCD
    // is where a live value belongs now.
    juce::TooltipWindow tooltipWindow{this};
};
