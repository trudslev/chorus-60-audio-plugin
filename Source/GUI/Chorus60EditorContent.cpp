#include "Chorus60EditorContent.h"
#include "../Parameters.h"

using namespace Chorus60Theme;

namespace
{
    const char* knobTooltip(const juce::String& paramID)
    {
        if (paramID == ParamIDs::rate1) return "Configuration I LFO rate.";
        if (paramID == ParamIDs::depth1) return "Configuration I modulation depth.";
        if (paramID == ParamIDs::center1) return "Configuration I BBD tap centre delay - offsets the scope trace.";
        if (paramID == ParamIDs::decorr1) return "Configuration I L/R decorrelation. No effect while set to MONO.";
        if (paramID == ParamIDs::image1) return "Configuration I stereo mode. STEREO inverts the right channel's modulation.";

        if (paramID == ParamIDs::rate2) return "Configuration II LFO rate.";
        if (paramID == ParamIDs::depth2) return "Configuration II modulation depth.";
        if (paramID == ParamIDs::center2) return "Configuration II BBD tap centre delay - offsets the scope trace.";
        if (paramID == ParamIDs::decorr2) return "Configuration II L/R decorrelation. No effect while set to MONO.";
        if (paramID == ParamIDs::image2) return "Configuration II stereo mode. STEREO inverts the right channel's modulation.";

        if (paramID == ParamIDs::rateB) return "Configuration I+II LFO rate - reaches far higher than I and II.";
        if (paramID == ParamIDs::depthB) return "Configuration I+II modulation depth.";
        if (paramID == ParamIDs::centerB) return "Configuration I+II BBD tap centre delay - offsets the scope trace.";
        if (paramID == ParamIDs::decorrB) return "Configuration I+II L/R decorrelation. Live, but inaudible until this page is set to STEREO.";
        if (paramID == ParamIDs::imageB) return "Configuration I+II stereo mode. Defaults to MONO, as the real circuit does.";
        if (paramID == ParamIDs::drift) return "Slow BBD clock wander, visible in the scope.";
        if (paramID == ParamIDs::saturation) return "BBD stage drive.";
        if (paramID == ParamIDs::noise) return "BBD clock noise floor, visible in the scope.";
        if (paramID == ParamIDs::mix) return "Dry/wet blend.";
        if (paramID == ParamIDs::trim) return "Output level trim, applied after the dry/wet mix.";
        return nullptr;
    }
}

ModSlotLabels::ModSlotLabels()
{
    // Pure overlay inside the MOD ENGINE group - the knobs and the switch own their own hit areas.
    setInterceptsMouseClicks(false, false);
}

void ModSlotLabels::setPage(const Layout::EnginePage& newPage)
{
    if (page == &newPage)
        return;
    page = &newPage;
    repaint();
}

void ModSlotLabels::paint(juce::Graphics& g)
{
    using namespace Chorus60Theme;

    // The five labels below the knob row: four slots plus IMAGE. They carry the page's suffix
    // ("DELAY CENTER I+II"), which is the whole reason they cannot be baked the way the five global
    // knob names are.
    //
    // Section 3: Barlow Condensed 600 at 12 CSS px, .18em. Centred on each control - measured off
    // chorus60-page-i@2x.png, where the five label centres land within 1.5 px of the knob centres
    // and the switch cell centre.
    const auto font = labelFont(labelFontHeightForCssPx(12.0f));
    const float tracking = trackingPxForEm(0.18f, 12.0f);

    const auto drawLabel = [&] (const juce::String& text, float centreX)
    {
        const juce::Rectangle<float> rect(centreX - Layout::modCellW * 0.5f, Layout::modLabelRowY,
                                           Layout::modCellW, Layout::modLabelRowH);
        drawTrackedText(g, text, font, tracking, rect, juce::Justification::centred,
                         Colour::controlLabelText);
    };

    for (int slot = 0; slot < 4; ++slot)
        drawLabel(juce::String(Layout::slotLabels[(size_t) slot]) + " " + page->suffix,
                   Layout::modKnobCentreX[(size_t) slot]);

    drawLabel(juce::String(Layout::imageSlotLabel) + " " + page->suffix,
               Layout::switchCellX + Layout::switchCellW * 0.5f);
}

Chorus60EditorContent::Chorus60EditorContent(Chorus60AudioProcessor& p)
    : processorRef(p),
      modEngineGroup(groupDimRect(Layout::modEngineGroupX, Layout::modEngineGroupY,
                                   Layout::modEngineGroupW, Layout::modEngineGroupH)),
      characterGroup(groupDimRect(Layout::characterGroupX, Layout::characterGroupY,
                                   Layout::characterGroupW, Layout::characterGroupH)),
      outputGroup(groupDimRect(Layout::outputGroupX, Layout::outputGroupY,
                                Layout::outputGroupW, Layout::outputGroupH)),
      buttonII(p, EngineButtonRole::engineII), buttonI(p, EngineButtonRole::engineI),
      buttonOff(p, EngineButtonRole::off),
      groupLed({Layout::modEngineLedCentreX - Layout::modEngineLedD * 0.5f,
                Layout::modEngineLedCentreY - Layout::modEngineLedD * 0.5f,
                Layout::modEngineLedD, Layout::modEngineLedD}),
      modScope(p), programHeader(p)
{
    setSize((int) Layout::canvasWidth, (int) Layout::canvasHeight);
    setLookAndFeel(&lookAndFeel);

    // Transparent: PluginEditor draws the plate behind this, and the three groups draw their own
    // crops of it so those regions can be multiplied down.
    setOpaque(false);

    for (auto* group : {&modEngineGroup, &characterGroup, &outputGroup})
        addAndMakeVisible(*group);

    slotLabels.setBounds(0, 0, (int) Layout::canvasWidth, (int) Layout::canvasHeight);
    modEngineGroup.content().addAndMakeVisible(slotLabels);

    // --- The five genuinely global knobs (CHARACTER + OUTPUT) ---------------------------------
    for (size_t i = 0; i < Layout::knobs.size(); ++i)
    {
        const auto& spec = Layout::knobs[i];

        auto knob = std::make_unique<KnobFilmstripComponent>(spec.size, spec.diameter);
        knob->setName(spec.paramID);

        // Bounds are the cap exactly. Revision 1 stretched them to the drawn tick ring's outer
        // radius; the ticks are printed on the plate now, and a hit area still reaching them would
        // swallow clicks on bare fascia and on the printed numerals.
        const float half = spec.diameter * 0.5f;
        knob->setBounds((int) std::round(spec.cx - half), (int) std::round(spec.cy - half),
                         (int) std::round(spec.diameter), (int) std::round(spec.diameter));

        if (const auto* tooltip = knobTooltip(spec.paramID))
            knob->setTooltip(tooltip);

        knobAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processorRef.apvts, spec.paramID, *knob);

        if (auto* param = processorRef.apvts.getParameter(spec.paramID))
            knob->setDoubleClickReturnValue(true, param->convertFrom0to1(param->getDefaultValue()));

        attachReadout(*knob, spec.paramID);

        // CHARACTER holds the first three, OUTPUT the last two - each has to live inside the group
        // whose multiply it belongs to.
        auto& host = i < 3 ? characterGroup : outputGroup;
        host.content().addAndMakeVisible(*knob);
        knobs[i] = std::move(knob);
    }

    // --- The paged MOD ENGINE box's four slots and its switch ---------------------------------
    //
    // Created once and re-attached on every page change. Recreating the components instead would
    // reset the very rotation the transition is supposed to animate away from, so the animation
    // would silently degrade into a snap.
    for (int slot = 0; slot < numSlots; ++slot)
    {
        auto knob = std::make_unique<KnobFilmstripComponent>(KnobFilmstripSize::mod, Layout::modKnobD);

        const float cx = Layout::modKnobCentreX[(size_t) slot];
        const float half = Layout::modKnobD * 0.5f;
        knob->setBounds((int) std::round(cx - half), (int) std::round(Layout::modKnobCentreY - half),
                         (int) std::round(Layout::modKnobD), (int) std::round(Layout::modKnobD));

        modEngineGroup.content().addAndMakeVisible(*knob);
        slotKnobs[(size_t) slot] = std::move(knob);
    }

    imageSwitch.setBounds((int) Layout::switchTrackX, (int) Layout::switchTrackY,
                           (int) Layout::switchTrackW, (int) Layout::switchTrackH);
    modEngineGroup.content().addAndMakeVisible(imageSwitch);

    bindPage(Layout::pageI);

    buttonII.setTooltip("Chorus Engine II - latch. I and II may be engaged together for the classic I+II sound.");
    buttonI.setTooltip("Chorus Engine I - latch. I and II may be engaged together for the classic I+II sound.");
    buttonOff.setTooltip("Disengage both chorus engines.");

    // The button column is outside every group: section 9 keeps it at full saturation, because on
    // the hardware the buttons are moulded plastic that never changes.
    for (auto* button : { &buttonII, &buttonI, &buttonOff })
    {
        button->setBounds(getLocalBounds());
        addAndMakeVisible(*button);
    }

    engine1Attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, ParamIDs::engine1, buttonI);
    engine2Attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, ParamIDs::engine2, buttonII);

    /*  **The engine buttons report to the LCD, like every knob.** BRAND.md's rule is that every
        control changing a parameter announces itself there, switches included - and these are its
        strongest case in the suite. Turning a knob shows you its own printed scale; pressing I or
        II rebinds what the entire MOD ENGINE page controls, and shows nothing about which
        configuration you have landed in.

        onClick rather than a value listener, so this fires only for a press: the attachment also
        moves these buttons on Program recall and on host automation, and neither should take the
        glass. Same distinction the knobs draw with isMouseButtonDown. */
    const auto reportEngine = [this] (juce::Button& button, const char* paramID)
    {
        button.onClick = [this, paramID]
        {
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (
                                   processorRef.apvts.getParameter (paramID)))
            {
                programHeader.showParameter (*ranged);
                programHeader.releaseParameter();
            }
        };
    };

    reportEngine (buttonI,  ParamIDs::engine1);
    reportEngine (buttonII, ParamIDs::engine2);

    // The heading-row LED sits ABOVE the MOD ENGINE box's rule, so it belongs to this component
    // rather than to the dimmable group - in bypass it goes dark rather than dimming.
    groupLed.setBounds(getLocalBounds());
    addAndMakeVisible(groupLed);

    // Bound to engine1 only as a formality - its real state is "any engine engaged", which the
    // animation timer keeps in step below. Binding it to one latch alone would leave it dark while
    // only II was engaged.
    groupLedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, ParamIDs::engine1, groupLed);

    // ModScope and ProgramHeader draw in absolute panel coordinates, so they are sized to the whole
    // content area rather than a sub-region - each narrows its own hitTest (ProgramHeader) or opts
    // out of mouse input entirely (ModScope) so they don't swallow clicks meant for the controls.
    // Both sit outside the group boxes and stay undimmed.
    modScope.setBounds(getLocalBounds());
    addAndMakeVisible(modScope);

    programHeader.setBounds(getLocalBounds());
    addAndMakeVisible(programHeader);

    // The Program list opens inside this, so it can neither move its top edge nor grow past the
    // panel. It must be a SIBLING of programHeader, never a child: that component narrows its
    // hitTest to the program window and the two buttons, and JUCE stops searching a component's
    // children once its own hitTest rejects the point - so a list parented there would be dead
    // everywhere except the cell it drops from.
    // **The foot is the chassis inset now, not the panel's edge.** The shared contract runs the
    // list to 16 px above the bottom — the same frame the header block observes on every other side
    // — rather than flush, so the list closes on the chassis instead of running off it.
    const int hostTop = ProgramHeader::menuHostTop();
    const int hostBottom = ProgramHeader::menuHostBottom(getHeight());
    menuHost.setBounds(0, hostTop, getWidth(), hostBottom - hostTop);
    menuHost.setInterceptsMouseClicks(false, true);
    addAndMakeVisible(menuHost);
    menuHost.toFront(false);
    programHeader.setMenuParent(&menuHost);

    // Seed at the settled position so the first frame after opening the editor is correct rather
    // than sweeping up from zero.
    const auto initial = processorRef.resolveActiveConfiguration();
    poweredDown = ! initial.engaged;
    dimFactor = poweredDown ? Layout::powerDownMultiply : 1.0f;
    for (auto* group : {&modEngineGroup, &characterGroup, &outputGroup})
        group->setDimFactor(dimFactor);
    for (int slot = 0; slot < numSlots; ++slot)
        slotDisplay[(size_t) slot] = slotKnobs[(size_t) slot]->getDrawnProportion();

    lastFrameMs = juce::Time::getMillisecondCounter();
    startTimerHz(60);
}

Chorus60EditorContent::~Chorus60EditorContent()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void Chorus60EditorContent::attachReadout(juce::Slider& knob, const juce::String& paramID)
{
    // Section 5: while a control is being moved the LCD's name cell shows its value. Guarded on the
    // knob's OWN drag state, never on the attachment's callback - a SliderAttachment also fires
    // when a Program is applied and on every host automation step, and without the guard the
    // display latches onto whichever parameter was written last and flickers for the length of a
    // song.
    auto* raw = &knob;
    auto* param = processorRef.apvts.getParameter(paramID);
    if (param == nullptr)
        return;

    // The same guard disarms the processor's stale-replay gate, because this is the only place that
    // knows a change came from a PERSON. It deliberately does not fire for automation: a host may
    // write automation on session load before replaying its remembered program index, and disarming
    // there would let that replay land on the restored state. One call rather than two adjacent
    // ones, so the disarm cannot be written without the hand-off - see nf/UserEditGate.h.
    nf::connectUserEdit(*raw, processorRef.userEdits,
                        [this, param] { programHeader.showParameter(*param); });
    knob.onDragEnd = [this] { programHeader.releaseParameter(); };
}

void Chorus60EditorContent::bindPage(const Layout::EnginePage& page)
{
    currentPage = &page;

    const char* ids[numSlots] = {page.rateID, page.depthID, page.centreID, page.decorrID};

    for (int slot = 0; slot < numSlots; ++slot)
    {
        auto& knob = *slotKnobs[(size_t) slot];
        const char* id = ids[slot];

        // Release the old attachment before making the new one: two live attachments on one Slider
        // would both write to their parameters on the next change.
        slotAttachments[(size_t) slot].reset();

        knob.setName(id);
        if (const auto* tooltip = knobTooltip(id))
            knob.setTooltip(tooltip);

        slotAttachments[(size_t) slot] =
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                processorRef.apvts, id, knob);

        if (auto* param = processorRef.apvts.getParameter(id))
            knob.setDoubleClickReturnValue(true, param->convertFrom0to1(param->getDefaultValue()));

        attachReadout(knob, id);
    }

    imageSwitchAttachment.reset();
    imageSwitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, page.imageID, imageSwitch);

    // The IMAGE switch reports through the LCD too, so a thrown switch reads "IMAGE I+II: MONO".
    if (auto* param = processorRef.apvts.getParameter(page.imageID))
    {
        imageSwitch.onClick = [this, param]
        {
            programHeader.showParameter(*param);
            programHeader.releaseParameter();
        };
    }

    slotLabels.setPage(page);
    repaint();
}

void Chorus60EditorContent::advanceAnimation(float dtMs)
{
    // Time-based coefficient, per section 10: 1 - 0.002^(dt/380ms). Travel then takes the same wall
    // time whatever the frame rate, and a dropped frame lengthens the step rather than shortening
    // the motion.
    const float slew = 1.0f - std::pow(Layout::slewRemainderAtSettle, dtMs / Layout::slewSettleMs);
    const float fade = 1.0f - std::pow(Layout::slewRemainderAtSettle, dtMs / Layout::powerDownFadeMs);

    // Polled against the current ProgramId rather than pushed from the manager: a Program can
    // arrive from the host, the dropdown or a session restore, and any unwired path would slew
    // silently.
    bool snapDisplay = false;

    if (const auto current = processorRef.getProgramManager().getCurrentProgramId();
        current != lastAppliedProgram)
    {
        lastAppliedProgram = current;
        snapDisplay = true;
    }

    for (int slot = 0; slot < numSlots; ++slot)
    {
        auto& knob = *slotKnobs[(size_t) slot];
        auto& display = slotDisplay[(size_t) slot];

        const float parameterProportion = (float) knob.valueToProportionOfLength(knob.getValue());

        // A slot being dragged tracks the pointer 1:1 - slewing under the user's own hand would
        // feel like lag, not like motion.
        //
        // **Program recall SNAPS; the paging slew survives**, and the distinction is not a
        // carve-out to save existing code. On recall the parameter's VALUE changes, so an animating
        // pointer shows a value that is not current for the length of the animation - the one thing
        // a pointer is for. On a page change the BINDING changes: the knob stops meaning slot A and
        // starts meaning slot B, no single current value is misreported, and the travel is what
        // tells you the knob now means something else. BRAND.md carries this beside the Program
        // conventions; do not tidy the two into one behaviour.
        if (snapDisplay || knob.isMouseButtonDown())
            display = parameterProportion;
        else
            display += slew * (parameterProportion - display);

        knob.setDisplayProportion(display);
    }

    // The global knobs never slew: they don't page, and section 9 deleted the wind-to-zero that
    // used to move them on bypass. They follow their parameters directly, always.

    const float dimTarget = poweredDown ? Layout::powerDownMultiply : 1.0f;
    dimFactor += fade * (dimTarget - dimFactor);

    for (auto* group : {&modEngineGroup, &characterGroup, &outputGroup})
        group->setDimFactor(dimFactor);
}

void Chorus60EditorContent::timerCallback()
{
    const auto now = juce::Time::getMillisecondCounter();
    const float dtMs = juce::jlimit(1.0f, 100.0f, (float) (now - lastFrameMs));
    lastFrameMs = now;

    // The latches are the pager - there is no other navigation control on the panel - so the page is
    // derived from the same resolver the audio thread uses rather than from a separate notion of
    // "current page" that could disagree with what is being heard.
    const auto active = processorRef.resolveActiveConfiguration();
    using Configuration = Chorus60AudioProcessor::Configuration;

    const bool nowPoweredDown = ! active.engaged;
    if (nowPoweredDown != poweredDown)
    {
        poweredDown = nowPoweredDown;
        imageSwitch.setPoweredDown(poweredDown);

        // Section 9: pointer interaction is disabled on all knobs and the switch; the LCD,
        // SAVE/DELETE and the engine buttons stay live.
        for (auto& knob : slotKnobs)
            if (knob != nullptr)
                knob->setInterceptsMouseClicks(! poweredDown, ! poweredDown);
        for (auto& knob : knobs)
            if (knob != nullptr)
                knob->setInterceptsMouseClicks(! poweredDown, ! poweredDown);

        repaint();
    }

    // While bypassed the last page is held: nothing is hidden and no page is "empty", so there is no
    // bypassed page to bind.
    if (active.engaged)
    {
        const auto& wanted = active.which == Configuration::both ? Layout::pageBoth
                            : active.which == Configuration::one ? Layout::pageI
                                                                 : Layout::pageII;
        if (currentPage != &wanted)
            bindPage(wanted);
    }

    groupLed.setToggleState(active.engaged, juce::dontSendNotification);

    advanceAnimation(dtMs);
}

void Chorus60EditorContent::paintOverChildren(juce::Graphics& g)
{
    using namespace Chorus60Theme;

    /*  **The nine printed rings.** They were pixels in the previous plate; the new one carries the
        fascia, the badge and the box frames only, so every tick and numeral is drawn here or is not
        drawn at all — and an absent ring is silent, where a doubled one used to be visible.

        Driven from `ringsToDraw()` rather than from a hand-written sequence, so the set the panel
        paints and the set the test walks are the same object. A pass that enumerated the knobs
        itself could omit one and still look complete. */
    for (const auto& ring : ringsToDraw())
        drawKnobScale (g, ring);

    // Only three strings are drawn out here, all of them above a heading rule or outside the boxes,
    // and all three because they carry live state. Everything else on the fascia is baked - see
    // design/CHORUS60-BUILD-HANDOFF.md section 1. The slot labels are NOT here: they sit below the
    // MOD ENGINE rule and so live inside that group's own dimmable subtree.

    // MOD ENGINE heading, section 3: Barlow Condensed 600 at 11 CSS px, .28em. Left edge measured
    // at x 322 off chorus60-page-i@2x.png, clear of the Ø8 LED at 308.5.
    const juce::Rectangle<float> headingRect(Layout::modEngineHeadingX, Layout::modEngineHeadingRowY,
                                              400.0f, Layout::modEngineHeadingRowH);
    drawTrackedText(g, currentPage->title, labelFont(labelFontHeightForCssPx(11.0f)),
                     trackingPxForEm(0.28f, 11.0f), headingRect, juce::Justification::centredLeft,
                     Colour::engravedHeadingText);

    // Status note, right-aligned to x 1237 - 20 px inside the box. Share Tech Mono 11 px / .06em,
    // in the caption grey, matching the scope's own status row.
    const char* note = poweredDown ? Layout::bypassStatusNote : currentPage->statusNote;
    const juce::Rectangle<float> noteRect(Layout::modEngineStatusRight - 500.0f,
                                           Layout::modEngineHeadingRowY, 500.0f,
                                           Layout::modEngineHeadingRowH);
    drawTrackedText(g, juce::String::fromUTF8(note), monoFont(monoFontHeightForCssPx(11.0f)),
                     trackingPxForEm(0.06f, 11.0f), noteRect, juce::Justification::right,
                     Colour::captionTertiary);

    // Footer. Drawn rather than baked because it names the engine state.
    // fromUTF8 on each literal, not on the assembled String: juce::String's char* constructor
    // assumes the platform default encoding, so a raw UTF-8 middot went in as two Latin-1
    // characters and came out as "Â·".
    const juce::String midDot = juce::String::fromUTF8("\xc2\xb7");
    const juce::String footer = "BBD 1024 STAGE " + midDot + " "
                              + (poweredDown ? "BYPASS" : "ENGAGED")
                              // **Derived, not a literal.** It read "v1.0" hard-coded, which was
                              // true only by coincidence until the suite went to 1.0.0 and would
                              // have quietly lied at the first 1.1. NF_VERSION_SHORT comes from
                              // PROJECT_VERSION in CMakeLists, so the panel and the plugin's
                              // reported version cannot disagree.
                              + " " + midDot + " v" NF_VERSION_SHORT;
    const juce::Rectangle<float> footerRect(Layout::footerRight - 400.0f, Layout::footerCentreY - 8.0f,
                                             400.0f, 16.0f);
    drawTrackedText(g, footer, monoFont(monoFontHeightForCssPx(10.0f)), trackingPxForEm(0.10f, 10.0f),
                     footerRect, juce::Justification::right, Colour::captionTertiary);
}
