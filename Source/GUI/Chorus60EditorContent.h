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
    One group box's whole printed layer: its heading, and the tick ring, numerals, unit and control
    label under every knob whose centre falls inside it.

    **This replaces `ModSlotLabels`, which existed to carry the page suffix.** §2.1 deleted the
    suffix — *"no panel text relabels itself on a page change"* — so the four paged labels became
    the same kind of string as the five global ones, and two drawing sites became one.

    **It lives inside the group's dimmable subtree, and that is the point.** §7.2 dims the whole
    box on OFF, so anything drawn over the group from outside — which is where `paintOverChildren`
    draws — stays at full brightness while the controls it belongs to fade. That is what the ring
    pass did before this: nine rings painted over three boxes that dim underneath them.

    Which knobs it owns is **filtered by containment, never listed**: see `ringsInBox`.
*/
class GroupPrintedLayer final : public juce::Component
{
public:
    explicit GroupPrintedLayer (const Chorus60Theme::Layout::GroupBox& box);

    /** Only MOD ENGINE's heading re-inks — §7.2 takes it to `#8a9196` on OFF, which is what
        replaced the deleted "BYPASS" status note. The other two never change. */
    void setBypassed (bool);

    /** What the last paint actually produced: rings drawn, and majors numeralled across them.
        Reported rather than assumed, so a pass that silently draws nothing is countable. */
    int ringsDrawn() const noexcept { return ringCount; }
    int numeralsDrawn() const noexcept { return numeralCount; }

private:
    void paint (juce::Graphics&) override;

    const Chorus60Theme::Layout::GroupBox& box;
    bool bypassed = false;
    int ringCount = 0, numeralCount = 0;
};

/**
    The assembler: owns every control plus its APVTS attachment, positions them from
    Chorus60Theme::Layout, and drives the page switch and the OFF state.

    Everything draws in inside-border panel coordinates on a fixed 1280 x 775 content area, which
    PluginEditor places at (1, 1) inside the 1282 x 777 plate and scales as a unit on resize.

    **THE STATIC-TEXT LAYER IS BACK, because the plate stopped carrying it.** Revision 2's plate
    baked every string whose characters and colour never change, which is why `PanelChrome` and
    `WordmarkComponent` were deleted rather than adapted. The revision-4 plate carries the fascia,
    the badge, the scope well and the three box frames — measured, not assumed — so those strings
    are drawn again, in `GroupPrintedLayer` (inside each box) and in `paintOverChildren` (outside
    them).

    The three group boxes are `DimmableGroup`s, not plain containers. §7.2's OFF state multiplies
    each box by 0.50, and a multiply has requirements about composition order that a container
    satisfies and a bare `setAlpha` does not - see DimmableGroup's class comment. Controls therefore
    go into `group.content()`, never onto this component directly — **and so does anything printed
    over them**, which is the correction the printed layer exists to hold.
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

    // One per box, each added to its own group's content so it dims with it. Order matches
    // Layout::groupBoxes.
    std::array<std::unique_ptr<GroupPrintedLayer>, Chorus60Theme::Layout::groupBoxes.size()> printedLayers;

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
    ProgramId lastAppliedProgram {};   // see Chorus60EditorContent.cpp: recall snaps, paging slews
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
