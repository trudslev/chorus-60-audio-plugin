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
        if (paramID == ParamIDs::mono1) return "Configuration I stereo mode. STEREO inverts the right channel's modulation.";

        if (paramID == ParamIDs::rate2) return "Configuration II LFO rate.";
        if (paramID == ParamIDs::depth2) return "Configuration II modulation depth.";
        if (paramID == ParamIDs::center2) return "Configuration II BBD tap centre delay - offsets the scope trace.";
        if (paramID == ParamIDs::decorr2) return "Configuration II L/R decorrelation. No effect while set to MONO.";
        if (paramID == ParamIDs::mono2) return "Configuration II stereo mode. STEREO inverts the right channel's modulation.";

        if (paramID == ParamIDs::rateB) return "Configuration I+II LFO rate - reaches far higher than I and II.";
        if (paramID == ParamIDs::depthB) return "Configuration I+II modulation depth.";
        if (paramID == ParamIDs::centerB) return "Configuration I+II BBD tap centre delay - offsets the scope trace.";
        if (paramID == ParamIDs::decorrB) return "Configuration I+II L/R decorrelation. Live, but inaudible until this page is set to STEREO.";
        if (paramID == ParamIDs::monoB) return "Configuration I+II stereo mode. Defaults to MONO, as the real circuit does.";
        if (paramID == ParamIDs::drift) return "Slow BBD clock wander, visible in the scope.";
        if (paramID == ParamIDs::saturation) return "BBD stage drive.";
        if (paramID == ParamIDs::noise) return "BBD clock noise floor, visible in the scope.";
        if (paramID == ParamIDs::mix) return "Dry/wet blend.";
        if (paramID == ParamIDs::trim) return "Output level trim, applied after the dry/wet mix.";
        return nullptr;
    }
}

KnobValueLabel::KnobValueLabel(juce::RangedAudioParameter& parameterToDisplay) : parameter(parameterToDisplay)
{
    setInterceptsMouseClicks(false, false);
    startTimerHz(20);
}

void KnobValueLabel::timerCallback()
{
    repaint();
}

void KnobValueLabel::paint(juce::Graphics& g)
{
    const double value = parameter.convertFrom0to1(parameter.getValue());
    g.setColour(Colour::valueText);
    g.setFont(monoFont(11.0f));
    g.drawText(formatParameterValue(parameter, value), getLocalBounds(), juce::Justification::centred, false);
}

Chorus60EditorContent::Chorus60EditorContent(Chorus60AudioProcessor& p)
    : processorRef(p),
      buttonII(p, EngineButtonRole::engineII), buttonI(p, EngineButtonRole::engineI), buttonOff(p, EngineButtonRole::off),
      groupLed(modEngineLedRect()),
      modScope(p), programHeader(p)
{
    setSize((int) Layout::canvasWidth, (int) Layout::canvasHeight);
    setLookAndFeel(&lookAndFeel);

    panelBackground.setBounds(getLocalBounds());
    addAndMakeVisible(panelBackground);

    panelChrome.setBounds(getLocalBounds());
    addAndMakeVisible(panelChrome);

    wordmark.setBounds(getLocalBounds());
    addAndMakeVisible(wordmark);

    for (size_t i = 0; i < Layout::knobs.size(); ++i)
    {
        const auto& spec = Layout::knobs[i];
        const float tickSpacing = spec.size == KnobFilmstripSize::large
                                       ? Layout::largeKnobTickSpacingDegrees
                                       : Layout::smallKnobTickSpacingDegrees;

        auto knob = std::make_unique<KnobFilmstripComponent>(spec.size, spec.diameter, tickSpacing);
        knob->setName(spec.paramID);

        // Bounding box reaches the tick ring's outer radius (+3px click margin), not just the
        // knob's own diameter - matches Gatecrasher's own convention.
        const float half = spec.diameter * 0.5f + Layout::tickOuterOffset + 3.0f;
        knob->setBounds((int) std::round(spec.cx - half), (int) std::round(spec.cy - half),
                         (int) std::round(half * 2.0f), (int) std::round(half * 2.0f));

        if (const auto* tooltip = knobTooltip(spec.paramID))
            knob->setTooltip(tooltip);

        knobAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processorRef.apvts, spec.paramID, *knob);

        if (auto* param = processorRef.apvts.getParameter(spec.paramID))
        {
            // Section 7: "double-click resets to default".
            knob->setDoubleClickReturnValue(true, param->convertFrom0to1(param->getDefaultValue()));

            // Live value popup while dragging, parented to this (not nullptr) so PluginEditor's
            // uniform scale transform applies to the popup too, same as every other on-canvas
            // element.
            knob->textFromValueFunction = [param](double value) { return formatParameterValue(*param, value); };
            knob->setPopupDisplayEnabled(true, false, this);

            // Permanent value line beneath the knob (see KnobValueLabel's own class comment).
            auto valueLabel = std::make_unique<KnobValueLabel>(*param);
            const float knobBottom = spec.cy + spec.diameter * 0.5f;
            const float valueY = knobBottom + Layout::knobLabelGap + Layout::knobNameRowH + Layout::knobLabelGap;
            valueLabel->setBounds((int) std::round(spec.cx - 50.0f), (int) std::round(valueY),
                                   100, (int) std::round(Layout::knobValueRowH));
            addAndMakeVisible(*valueLabel);
            knobValueLabels[i] = std::move(valueLabel);
        }

        addAndMakeVisible(*knob);
        knobs[i] = std::move(knob);
    }

    // --- The paged MOD ENGINE box's four slots and its switch -----------------------------
    //
    // Created once and re-attached on every page change. Recreating the components instead would
    // reset the very rotation the transition is supposed to animate away from, so the animation
    // would silently degrade into a snap.
    for (int slot = 0; slot < numSlots; ++slot)
    {
        auto knob = std::make_unique<KnobFilmstripComponent>(KnobFilmstripSize::large,
                                                              Layout::slotWellD,
                                                              Layout::largeKnobTickSpacingDegrees);

        const float cx = Layout::slotCentreX(slot);
        const float half = Layout::slotWellD * 0.5f + Layout::tickOuterOffset + 3.0f;
        knob->setBounds((int) std::round(cx - half), (int) std::round(Layout::slotCentreY - half),
                         (int) std::round(half * 2.0f), (int) std::round(half * 2.0f));
        knob->setPopupDisplayEnabled(true, false, this);

        addAndMakeVisible(*knob);
        slotKnobs[(size_t) slot] = std::move(knob);
    }

    monoSwitch.setBounds((int) Layout::switchX, (int) Layout::switchY,
                          (int) Layout::switchW, (int) Layout::switchH);
    addAndMakeVisible(monoSwitch);

    bindPage(Layout::pageI);

    buttonII.setTooltip("Chorus Engine II - latch. I and II may be engaged together for the classic I+II sound.");
    buttonI.setTooltip("Chorus Engine I - latch. I and II may be engaged together for the classic I+II sound.");
    buttonOff.setTooltip("Disengage both chorus engines.");

    for (auto* button : { &buttonII, &buttonI, &buttonOff })
    {
        button->setBounds(getLocalBounds());
        addAndMakeVisible(*button);
    }

    engine1Attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, ParamIDs::engine1, buttonI);
    engine2Attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, ParamIDs::engine2, buttonII);

    groupLed.setBounds(getLocalBounds());
    addAndMakeVisible(groupLed);

    // Bound to engine1 only as a formality - the LED's real state is "any engine engaged", which
    // the animation timer keeps in step below. Binding it to one latch alone would leave it dark
    // while only II was engaged.
    groupLedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, ParamIDs::engine1, groupLed);

    // ModScope/ProgramHeader draw with absolute canvas coordinates (like
    // Chorus60PanelBackground/WordmarkComponent), so they're sized to the full canvas rather than a
    // sub-region - each narrows its own hitTest (ProgramHeader) or opts out of mouse input entirely
    // (ModScope) so they don't swallow clicks meant for the knobs/buttons.
    modScope.setBounds(getLocalBounds());
    addAndMakeVisible(modScope);

    programHeader.setBounds(getLocalBounds());
    addAndMakeVisible(programHeader);

    // Seed the animation at its settled position so the very first frame after opening the editor
    // is correct rather than sweeping up from zero.
    const auto initial = processorRef.resolveActiveConfiguration();
    poweredDown = ! initial.engaged;
    powerFactor = poweredDown ? 0.0f : 1.0f;
    fadeFactor = poweredDown ? Layout::powerDownOpacity : 1.0f;
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
        slotValueLabels[(size_t) slot].reset();

        knob.setName(id);
        if (const auto* tooltip = knobTooltip(id))
            knob.setTooltip(tooltip);

        slotAttachments[(size_t) slot] =
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                processorRef.apvts, id, knob);

        if (auto* param = processorRef.apvts.getParameter(id))
        {
            knob.setDoubleClickReturnValue(true, param->convertFrom0to1(param->getDefaultValue()));
            knob.textFromValueFunction = [param](double value) { return formatParameterValue(*param, value); };

            auto valueLabel = std::make_unique<KnobValueLabel>(*param);
            const float knobBottom = Layout::slotCentreY + Layout::slotWellD * 0.5f;
            const float valueY = knobBottom + Layout::knobLabelGap + Layout::knobNameRowH + Layout::knobLabelGap;
            valueLabel->setBounds((int) std::round(Layout::slotCentreX(slot) - 60.0f),
                                   (int) std::round(valueY),
                                   120, (int) std::round(Layout::knobValueRowH));
            addAndMakeVisible(*valueLabel);
            slotValueLabels[(size_t) slot] = std::move(valueLabel);
        }
    }

    monoSwitchAttachment.reset();
    monoSwitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, page.monoID, monoSwitch);

    repaint();
}

void Chorus60EditorContent::advanceAnimation(float dtMs)
{
    // Time-based coefficient, per spec section 7a: 1 - 0.002^(dt/380ms). Travel then takes the same
    // wall time whatever the frame rate, and a dropped frame lengthens the step rather than
    // shortening the motion.
    const float slew = 1.0f - std::pow(Layout::slewRemainderAtSettle, dtMs / Layout::slewSettleMs);
    const float fade = 1.0f - std::pow(Layout::slewRemainderAtSettle, dtMs / Layout::powerDownFadeMs);

    for (int slot = 0; slot < numSlots; ++slot)
    {
        auto& knob = *slotKnobs[(size_t) slot];
        auto& display = slotDisplay[(size_t) slot];

        const float parameterProportion = (float) knob.valueToProportionOfLength(knob.getValue());

        // A slot being dragged tracks the pointer 1:1 - slewing under the user's own hand would
        // feel like lag, not like motion.
        if (knob.isMouseButtonDown())
        {
            display = parameterProportion;
        }
        else
        {
            // Powered down, every knob winds to minimum. This moves only what is *drawn*: the
            // parameters keep their values, because the panel reads SETTINGS RETAINED and powering
            // back up has to return to exactly what was there.
            const float target = poweredDown ? 0.0f : parameterProportion;
            display += slew * (target - display);
        }

        knob.setDisplayProportion(display);
        knob.setDimFactor(fadeFactor);
    }

    // One shared power factor for the Character/Output knobs so the lower panel winds down
    // together rather than at five independent rates.
    const float powerTarget = poweredDown ? 0.0f : 1.0f;
    powerFactor += slew * (powerTarget - powerFactor);

    const float fadeTarget = poweredDown ? Layout::powerDownOpacity : 1.0f;
    fadeFactor += fade * (fadeTarget - fadeFactor);

    for (auto& knob : knobs)
    {
        if (knob == nullptr)
            continue;

        const float parameterProportion = (float) knob->valueToProportionOfLength(knob->getValue());
        if (knob->isMouseButtonDown())
            knob->clearDisplayProportion();
        else
            knob->setDisplayProportion(parameterProportion * powerFactor);

        knob->setDimFactor(fadeFactor);
    }
}

void Chorus60EditorContent::timerCallback()
{
    const auto now = juce::Time::getMillisecondCounter();
    const float dtMs = juce::jlimit(1.0f, 100.0f, (float) (now - lastFrameMs));
    lastFrameMs = now;

    // The latches are the pager - there is no other navigation control on the panel - so the page
    // is derived from the same resolver the audio thread uses rather than from a separate notion of
    // "current page" that could disagree with what is being heard.
    const auto active = processorRef.resolveActiveConfiguration();
    using Configuration = Chorus60AudioProcessor::Configuration;

    const bool nowPoweredDown = ! active.engaged;
    if (nowPoweredDown != poweredDown)
    {
        poweredDown = nowPoweredDown;
        monoSwitch.setPoweredDown(poweredDown);

        for (auto& knob : slotKnobs)
            if (knob != nullptr)
                knob->setInterceptsMouseClicks(! poweredDown, ! poweredDown);
        for (auto& knob : knobs)
            if (knob != nullptr)
                knob->setInterceptsMouseClicks(! poweredDown, ! poweredDown);
    }

    // While bypassed the last page is held: nothing is hidden and no page is "empty", so there is
    // no bypassed page to bind.
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
    // The MOD ENGINE box's title row is live rather than engraved into the background: it names the
    // engaged configuration and carries a right-aligned note, both of which change with the
    // latches. Everything else in the title row (the boxes themselves, the static group names) is
    // baked, so only these two strings are drawn here.
    using namespace Chorus60Theme;

    const float titleX = Layout::modEngineGroupX + Layout::groupTitleInsetXWithLed;
    const float titleY = Layout::modEngineGroupY + Layout::groupTitleCentreBelowTop;

    const juce::Colour titleColour = poweredDown ? Colour::inactiveLabel : Colour::engravedHeadingText;
    const juce::Rectangle<float> titleRect(titleX, titleY - 8.0f, 400.0f, 16.0f);

    drawTrackedText(g, currentPage->title, labelFontBold(11.0f), 11.0f * 0.18f, titleRect,
                     juce::Justification::centredLeft, titleColour);

    const char* note = poweredDown ? Layout::bypassHeaderNote : currentPage->headerNote;
    const juce::Rectangle<float> noteRect(Layout::modEngineGroupX + Layout::modEngineGroupW - 420.0f,
                                           titleY - 8.0f, 402.0f, 16.0f);
    g.setFont(monoFont(10.0f));
    g.setColour(Colour::inactiveLabel);
    g.drawText(juce::String::fromUTF8(note), noteRect, juce::Justification::centredRight, false);

    // Slot labels carry the page's suffix ("DELAY CENTER I+II"), so they cannot be baked either.
    for (int slot = 0; slot < numSlots; ++slot)
    {
        const float cx = Layout::slotCentreX(slot);
        const float nameY = Layout::slotCentreY + Layout::slotWellD * 0.5f + Layout::knobLabelGap;
        const juce::Rectangle<float> nameRect(cx - 90.0f, nameY, 180.0f, Layout::knobNameRowH);

        const juce::String label = juce::String(Layout::slotLabels[(size_t) slot])
                                 + " " + currentPage->suffix;
        drawTrackedText(g, label, labelFont(10.0f), 10.0f * 0.18f, nameRect,
                         juce::Justification::centred,
                         poweredDown ? Colour::inactiveLabel : Colour::controlLabelText);
    }
}
