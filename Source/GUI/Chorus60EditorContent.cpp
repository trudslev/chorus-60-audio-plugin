#include "Chorus60EditorContent.h"
#include "../Parameters.h"

using namespace Chorus60Theme;

namespace
{
    const char* knobTooltip(const juce::String& paramID)
    {
        if (paramID == ParamIDs::rate1) return "Engine I LFO rate.";
        if (paramID == ParamIDs::depth1) return "Engine I modulation depth.";
        if (paramID == ParamIDs::rate2) return "Engine II LFO rate.";
        if (paramID == ParamIDs::depth2) return "Engine II modulation depth.";
        if (paramID == ParamIDs::delayCenter) return "BBD tap centre delay - offsets the scope trace.";
        if (paramID == ParamIDs::decorrelation) return "L/R modulator phase offset: 0% mono-linked, 100% 180deg apart.";
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
      groupLedI(modEngineILedRect()), groupLedII(modEngineIILedRect()),
      modScope(p), programHeader(p)
{
    setSize((int) Layout::canvasWidth, (int) Layout::canvasHeight);
    setLookAndFeel(&lookAndFeel);

    panelBackground.setBounds(getLocalBounds());
    addAndMakeVisible(panelBackground);

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

    groupLedI.setBounds(getLocalBounds());
    addAndMakeVisible(groupLedI);
    groupLedII.setBounds(getLocalBounds());
    addAndMakeVisible(groupLedII);

    groupLedIAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, ParamIDs::engine1, groupLedI);
    groupLedIIAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, ParamIDs::engine2, groupLedII);

    // ModScope/ProgramHeader draw with absolute canvas coordinates (like
    // Chorus60PanelBackground/WordmarkComponent), so they're sized to the full canvas rather than a
    // sub-region - each narrows its own hitTest (ProgramHeader) or opts out of mouse input entirely
    // (ModScope) so they don't swallow clicks meant for the knobs/buttons.
    modScope.setBounds(getLocalBounds());
    addAndMakeVisible(modScope);

    programHeader.setBounds(getLocalBounds());
    addAndMakeVisible(programHeader);
}

Chorus60EditorContent::~Chorus60EditorContent()
{
    setLookAndFeel(nullptr);
}
