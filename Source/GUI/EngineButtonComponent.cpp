#include "EngineButtonComponent.h"
#include "Chorus60Theme.h"
#include "../Parameters.h"

namespace
{
    using namespace Chorus60Theme;

    struct RoleStyle
    {
        juce::Rectangle<float> faceRect;
        juce::Colour top, bottom;
        float highlightAlpha;
        juce::Rectangle<float> ledRect; // empty for OFF (no LED)
        const char* label;
        float labelFontPx;
        float labelTrackingEm;
    };

    RoleStyle styleFor(EngineButtonRole role)
    {
        using namespace Layout;
        switch (role)
        {
            case EngineButtonRole::engineII:
                return { {buttonIIX, buttonIIY, buttonW, buttonH}, Colour::buttonIITop, Colour::buttonIIBottom,
                         0.40f, {ledIIX, ledIIY, ledD, ledD}, "II", 22.0f, 0.06f };
            case EngineButtonRole::engineI:
                return { {buttonIX, buttonIY, buttonW, buttonH}, Colour::buttonITop, Colour::buttonIBottom,
                         0.50f, {ledIX, ledIY, ledD, ledD}, "I", 22.0f, 0.06f };
            case EngineButtonRole::off:
            default:
                return { {buttonOffX, buttonOffY, buttonW, buttonH}, Colour::buttonOffTop, Colour::buttonOffBottom,
                         0.85f, {}, "OFF", 18.0f, 0.14f };
        }
    }

    // Shared by both EngineButtonComponent's own Ø15 LEDs and EngineLedIndicator's Ø8 group-panel
    // echo - same gradients/glow either way (section 4), just a different radius.
    void drawEngineLed(juce::Graphics& g, juce::Rectangle<float> bounds, bool lit)
    {
        const auto centre = bounds.getCentre();
        const float r = bounds.getWidth() * 0.5f;

        if (lit)
        {
            // Glow halo: "0 0 12px 3px rgba(255,43,28,.55) and 0 0 30px 8px rgba(255,43,28,.22)".
            const float glowR = r + 22.0f;
            juce::ColourGradient glow(Colour::chorusAccent.withAlpha(0.55f), centre.x, centre.y,
                                       Colour::chorusAccent.withAlpha(0.0f), centre.x + glowR, centre.y, true);
            glow.addColour(0.25, Colour::chorusAccent.withAlpha(0.35f));
            glow.addColour(0.55, Colour::chorusAccent.withAlpha(0.18f));
            glow.addColour(0.8, Colour::chorusAccent.withAlpha(0.06f));
            g.setGradientFill(glow);
            g.fillEllipse(centre.x - glowR, centre.y - glowR, glowR * 2.0f, glowR * 2.0f);

            // Lit radial: #FF2B1C -> #B0140C @70% -> #6D0B06.
            juce::ColourGradient bulb(Colour::ledLitCore, centre.x, centre.y,
                                       Colour::ledLitEdge, centre.x + r, centre.y + r, true);
            bulb.addColour(0.7, Colour::ledLitMid);
            g.setGradientFill(bulb);
            g.fillEllipse(bounds);
        }
        else
        {
            g.setColour(Colour::ledUnlit);
            g.fillEllipse(bounds);
            g.setColour(juce::Colours::white.withAlpha(0.14f));
            g.drawEllipse(bounds.reduced(1.0f), 1.2f);
        }
    }

    void drawButtonFace(juce::Graphics& g, juce::Rectangle<float> rect, juce::Colour top, juce::Colour bottom,
                         float highlightAlpha)
    {
        juce::Path roundedPath;
        roundedPath.addRoundedRectangle(rect, Layout::buttonCornerRadius);

        // Drop shadow, section 4: "0 7px 13px -7px rgba(0,0,0,.95)".
        juce::DropShadow shadow(juce::Colours::black.withAlpha(0.55f), 9, {0, 5});
        shadow.drawForPath(g, roundedPath);

        g.setGradientFill(angledGradient(rect, top, bottom, 160.0f));
        g.fillPath(roundedPath);

        g.saveState();
        g.reduceClipRegion(roundedPath);

        // Inner bottom shade, section 4: "0 -7px 12px -4px rgba(0,0,0,.35-.50)".
        juce::ColourGradient innerShade(juce::Colours::transparentBlack, rect.getCentreX(), rect.getBottom() - 26.0f,
                                         juce::Colours::black.withAlpha(0.42f), rect.getCentreX(), rect.getBottom(), false);
        g.setGradientFill(innerShade);
        g.fillRect(rect);

        // Top inner highlight, section 4: 1px line just inside the top edge, alpha varies per role.
        g.setColour(juce::Colours::white.withAlpha(highlightAlpha));
        g.drawLine(rect.getX() + 3.0f, rect.getY() + 1.5f, rect.getRight() - 3.0f, rect.getY() + 1.5f, 1.0f);

        g.restoreState();
    }
}

EngineButtonComponent::EngineButtonComponent(Chorus60AudioProcessor& processor, EngineButtonRole roleIn)
    : juce::Button({}), processorRef(processor), role(roleIn)
{
    setInterceptsMouseClicks(true, false);

    if (role == EngineButtonRole::off)
    {
        engine1Raw = processorRef.apvts.getRawParameterValue(ParamIDs::engine1);
        engine2Raw = processorRef.apvts.getRawParameterValue(ParamIDs::engine2);

        // No APVTS binding for OFF itself - it directly clears both engine parameters. Wrapped in
        // begin/endChangeGesture so hosts record this as a proper automation gesture on each
        // parameter, the same as a normal click-drag would.
        onClick = [this]
        {
            if (auto* p1 = processorRef.apvts.getParameter(ParamIDs::engine1))
            {
                p1->beginChangeGesture();
                p1->setValueNotifyingHost(0.0f);
                p1->endChangeGesture();
            }
            if (auto* p2 = processorRef.apvts.getParameter(ParamIDs::engine2))
            {
                p2->beginChangeGesture();
                p2->setValueNotifyingHost(0.0f);
                p2->endChangeGesture();
            }
        };
    }

    startTimerHz(60);
}

EngineButtonComponent::~EngineButtonComponent()
{
    stopTimer();
}

bool EngineButtonComponent::hitTest(int x, int y)
{
    return styleFor(role).faceRect.contains((float) x, (float) y);
}

void EngineButtonComponent::timerCallback()
{
    using namespace Chorus60Theme::Layout;

    // Time-based linear ease so the 3px travel completes in exactly pressAnimMs regardless of
    // frame-rate jitter (section 4: "translate down 3px for 110ms, then release").
    const float target = isDown() ? Chorus60Theme::Layout::pressOffsetPx : 0.0f;
    const float maxStep = Chorus60Theme::Layout::pressOffsetPx / (pressAnimMs / (1000.0f / 60.0f));

    if (pressOffsetPx < target)
        pressOffsetPx = juce::jmin(target, pressOffsetPx + maxStep);
    else if (pressOffsetPx > target)
        pressOffsetPx = juce::jmax(target, pressOffsetPx - maxStep);

    // Continuous repaint (rather than only on change) also keeps the OFF button's derived
    // label-brightness state fresh, since that's re-evaluated from the raw parameters every
    // paintButton() call rather than cached.
    repaint();
}

void EngineButtonComponent::paintButton(juce::Graphics& g, bool, bool)
{
    using namespace Chorus60Theme;

    const auto style = styleFor(role);

    g.saveState();
    g.addTransform(juce::AffineTransform::translation(0.0f, pressOffsetPx));

    drawButtonFace(g, style.faceRect, style.top, style.bottom, style.highlightAlpha);

    bool labelBright;
    juce::Rectangle<float> labelRect;
    juce::Colour dimColour;

    if (role == EngineButtonRole::off)
    {
        const bool engine1On = engine1Raw != nullptr && engine1Raw->load(std::memory_order_relaxed) > 0.5f;
        const bool engine2On = engine2Raw != nullptr && engine2Raw->load(std::memory_order_relaxed) > 0.5f;
        labelBright = !(engine1On || engine2On);
        labelRect = { Layout::engineLabelX, style.faceRect.getCentreY() - 15.0f, Layout::engineLabelW, 30.0f };
        dimColour = Colour::captionTertiary;
    }
    else
    {
        const bool lit = getToggleState();
        drawEngineLed(g, style.ledRect, lit);
        labelBright = lit;
        labelRect = { Layout::engineLabelX, style.ledRect.getCentreY() - 15.0f, Layout::engineLabelW, 30.0f };
        dimColour = Colour::controlLabelText;
    }

    const auto labelColour = labelBright ? Colour::engravedHeadingText : dimColour;
    drawTrackedText(g, style.label, labelFontBold(style.labelFontPx), style.labelFontPx * style.labelTrackingEm,
                     labelRect, juce::Justification::centredLeft, labelColour);

    g.restoreState();
}

EngineLedIndicator::EngineLedIndicator(juce::Rectangle<float> ledBoundsAbsolute)
    : juce::Button({}), ledBounds(ledBoundsAbsolute)
{
    // Purely passive/decorative - never intercepts clicks, so it never steals a click meant for
    // whatever's beneath it. Its toggle state is driven entirely by its own ButtonAttachment.
    setInterceptsMouseClicks(false, false);
}

void EngineLedIndicator::paintButton(juce::Graphics& g, bool, bool)
{
    drawEngineLed(g, ledBounds, getToggleState());
}
