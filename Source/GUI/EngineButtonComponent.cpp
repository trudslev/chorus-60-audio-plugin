#include "EngineButtonComponent.h"
#include "Chorus60Theme.h"
#include "../Parameters.h"

namespace
{
    using namespace Chorus60Theme;

    struct RoleStyle
    {
        juce::Rectangle<float> faceRect;
        int spriteIndex;                    // engineButtonImage(): 0 = II, 1 = I, 2 = OFF
        juce::Point<float> lampCentre;      // (0,0) for OFF - the hardware has no lamp there
        bool hasLamp;
        const char* label;
        float labelCssPx;
        float labelTrackingEm;
    };

    RoleStyle styleFor(EngineButtonRole role)
    {
        using namespace Layout;
        switch (role)
        {
            case EngineButtonRole::engineII:
                return { {buttonX, buttonIIY, buttonW, buttonH}, 0,
                         {lampIICentreX, lampIICentreY}, true, "II", 22.0f, 0.06f };
            case EngineButtonRole::engineI:
                return { {buttonX, buttonIY, buttonW, buttonH}, 1,
                         {lampICentreX, lampICentreY}, true, "I", 22.0f, 0.06f };
            case EngineButtonRole::off:
            default:
                return { {buttonX, buttonOffY, buttonW, buttonH}, 2,
                         {}, false, "OFF", 18.0f, 0.14f };
        }
    }

    // Section 3 of the handoff: the seating shadow is deliberately NOT baked into the button
    // sprites, because a baked shadow fights the plate's own material. Drawn here instead, to the
    // spec's `0 7px 13px -7px rgba(0,0,0,.95)`.
    void drawSeatingShadow(juce::Graphics& g, juce::Rectangle<float> rect)
    {
        juce::Path p;
        p.addRoundedRectangle(rect.reduced(7.0f).translated(0.0f, 7.0f), 5.0f);
        juce::DropShadow(juce::Colours::black.withAlpha(0.95f), 13, {}).drawForPath(g, p);
    }

    void drawLampSprite(juce::Graphics& g, juce::Point<float> centre, bool lit)
    {
        // 96 x 96 sprite with the glow baked into its transparent margin, so it is placed by its
        // CENTRE - the visible bulb is only 15 px of it. Registering it by top-left would put the
        // bulb 40 px down and right of where the panel prints it.
        const float half = Layout::lampSpriteD * 0.5f;
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        g.drawImage(lampImage(lit),
                     (int) std::round(centre.x - half), (int) std::round(centre.y - half),
                     (int) Layout::lampSpriteD, (int) Layout::lampSpriteD,
                     0, 0, lampImage(lit).getWidth(), lampImage(lit).getHeight());
    }
}

EngineButtonComponent::EngineButtonComponent(Chorus60AudioProcessor& processor, EngineButtonRole roleIn)
    : juce::Button({}), processorRef(processor), role(roleIn)
{
    setInterceptsMouseClicks(true, false);

    // The engine engages on the way DOWN, not on release - that's how the real JN-80's latching
    // switches behave: the lamp is already lit by the time the cap bottoms out. Waiting for mouse-up
    // (JUCE's default) puts the state change after the travel, which reads as lag on a control whose
    // whole character is that it's instant.
    setTriggeredOnMouseDown(true);

    if (role != EngineButtonRole::off)
    {
        // Latching, not momentary. Without this the button never changes its own toggle state on a
        // click, so the ButtonAttachment bound to engine1/engine2 has nothing to observe and the
        // parameter is never written - i.e. the engines simply cannot be switched on from the panel.
        // OFF is deliberately excluded: it IS momentary, and clears both engines through its onClick
        // below rather than carrying a state of its own.
        setClickingTogglesState(true);
    }

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
    // frame-rate jitter (section 4: "translates the button +3 px in y for 110 ms").
    const float target = isDown() ? Chorus60Theme::Layout::pressOffsetPx : 0.0f;
    const float maxStep = Chorus60Theme::Layout::pressOffsetPx / (pressAnimMs / (1000.0f / 60.0f));

    if (pressOffset < target)
        pressOffset = juce::jmin(target, pressOffset + maxStep);
    else if (pressOffset > target)
        pressOffset = juce::jmax(target, pressOffset - maxStep);

    // Continuous repaint (rather than only on change) also keeps the OFF button's derived
    // label-brightness state fresh, since that's re-evaluated from the raw parameters every
    // paintButton() call rather than cached.
    repaint();
}

void EngineButtonComponent::paintButton(juce::Graphics& g, bool, bool)
{
    using namespace Chorus60Theme;

    const auto style = styleFor(role);

    // Section 3: the button faces are state-independent - there is no lit/unlit pair, because on the
    // JN-80 they are plain moulded plastic that never illuminates. The LAMP beside each one carries
    // the state, which is why lamp-on/lamp-off ship as a pair and the buttons do not.
    //
    // Only the face travels on a press. The lamp and the roman legend sit outside it - the buttons
    // end at x 158 and the lamps are centred at 182.5 - so they are panel furniture the button is
    // pressed *next to*, not printed on. Sinking them with it made the whole assembly look like one
    // flexing sheet.
    {
        juce::Graphics::ScopedSaveState pressedFace(g);
        g.addTransform(juce::AffineTransform::translation(0.0f, pressOffset));

        const auto& sprite = engineButtonImage(style.spriteIndex);
        drawSeatingShadow(g, style.faceRect);
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        g.drawImage(sprite,
                     (int) style.faceRect.getX(), (int) style.faceRect.getY(),
                     (int) style.faceRect.getWidth(), (int) style.faceRect.getHeight(),
                     0, 0, sprite.getWidth(), sprite.getHeight());
    }

    // The letters are drawn rather than baked because their colour follows engagement, and section 9
    // is explicit that this is the ONE per-element colour change in the whole OFF state - a state
    // readout the live panel already has, not a dimming effect. In bypass nothing is engaged, so I
    // and II read #A5ADB2 and OFF reads #E6EBEE.
    bool engaged;
    juce::Rectangle<float> labelRect;

    if (role == EngineButtonRole::off)
    {
        const bool engine1On = engine1Raw != nullptr && engine1Raw->load(std::memory_order_relaxed) > 0.5f;
        const bool engine2On = engine2Raw != nullptr && engine2Raw->load(std::memory_order_relaxed) > 0.5f;
        engaged = !(engine1On || engine2On);
        labelRect = { Layout::engineLetterX, style.faceRect.getCentreY() - 16.0f, Layout::engineLetterW, 32.0f };
    }
    else
    {
        engaged = getToggleState();
        drawLampSprite(g, style.lampCentre, engaged);
        labelRect = { Layout::engineLetterX, style.lampCentre.y - 16.0f, Layout::engineLetterW, 32.0f };
    }

    drawTrackedText(g, style.label, labelFontBold(labelFontHeightForCssPx(style.labelCssPx)),
                     trackingPxForEm(style.labelTrackingEm, style.labelCssPx), labelRect,
                     juce::Justification::centredLeft,
                     engaged ? Colour::engravedHeadingText : Colour::controlLabelText);
}

EngineLedIndicator::EngineLedIndicator(juce::Rectangle<float> ledBoundsAbsolute)
    : juce::Button({}), ledBounds(ledBoundsAbsolute)
{
    // Purely passive - never intercepts clicks, so it never steals one meant for whatever is
    // beneath it. Its toggle state is driven entirely by its own ButtonAttachment.
    setInterceptsMouseClicks(false, false);
}

void EngineLedIndicator::paintButton(juce::Graphics& g, bool, bool)
{
    using namespace Chorus60Theme;

    // The MOD ENGINE box's own Ø8 heading-row indicator, at (308.5, 268.5) measured off
    // chorus60-page-i@2x.png. It sits ABOVE the heading rule, so it never dims with the OFF state -
    // it simply goes dark, which is why the unlit case is drawn flat rather than at reduced alpha.
    if (getToggleState())
    {
        const auto centre = ledBounds.getCentre();
        const float r = ledBounds.getWidth() * 0.5f;
        const float glowR = r + 9.0f;

        juce::ColourGradient glow(Colour::chorusAccent.withAlpha(0.50f), centre.x, centre.y,
                                   Colour::chorusAccent.withAlpha(0.0f), centre.x + glowR, centre.y, true);
        glow.addColour(0.45, Colour::chorusAccent.withAlpha(0.22f));
        g.setGradientFill(glow);
        g.fillEllipse(centre.x - glowR, centre.y - glowR, glowR * 2.0f, glowR * 2.0f);

        g.setColour(Colour::chorusAccent);
        g.fillEllipse(ledBounds);
    }
    else
    {
        g.setColour(juce::Colour(0xFF3A1512));
        g.fillEllipse(ledBounds);
    }
}
