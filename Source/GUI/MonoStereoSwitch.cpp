#include "MonoStereoSwitch.h"
#include <cmath>

namespace
{
    using namespace Chorus60Theme;

    // Section 7a's own values.
    constexpr float trackCornerRadius = 17.0f;
    constexpr float thumbDiameter = 26.0f;
    constexpr float thumbInset = 4.0f;
    constexpr float thumbTravel = 24.0f;
    constexpr float travelMs = 260.0f;

    const juce::Colour trackTop{0xFF0A0C0D};
    const juce::Colour trackBottom{0xFF141719};
    const juce::Colour thumbTop{0xFFE4E8EA};
    const juce::Colour thumbBottom{0xFFA9B0B5};

    // cubic-bezier(.3, 1.5, .5, 1) - the overshoot is the point: the thumb passes its target and
    // settles back, which is what makes a two-position switch read as sprung rather than as a
    // value fading between two states. Solved by bisection on x because a CSS timing function is
    // parameterised by t, not by progress.
    float springEase(float x)
    {
        constexpr float x1 = 0.3f, y1 = 1.5f, x2 = 0.5f, y2 = 1.0f;
        const auto bezier = [] (float a, float b, float t)
        {
            const float u = 1.0f - t;
            return 3.0f * u * u * t * a + 3.0f * u * t * t * b + t * t * t;
        };

        float lo = 0.0f, hi = 1.0f, t = x;
        for (int i = 0; i < 16; ++i)
        {
            const float guess = bezier(x1, x2, t);
            if (guess < x) lo = t; else hi = t;
            t = 0.5f * (lo + hi);
        }
        return bezier(y1, y2, t);
    }
}

MonoStereoSwitch::MonoStereoSwitch() : juce::Button("MonoStereoSwitch")
{
    setClickingTogglesState(true);
    setWantsKeyboardFocus(false);
}

MonoStereoSwitch::~MonoStereoSwitch()
{
    stopTimer();
}

void MonoStereoSwitch::setPoweredDown(bool shouldBePoweredDown)
{
    if (poweredDown == shouldBePoweredDown)
        return;

    poweredDown = shouldBePoweredDown;
    // Section 7a: non-interactive while powered down, cursor `default`, drags rejected. The switch
    // keeps its position and stays legible - this is a power-down, not a hide.
    setInterceptsMouseClicks(! poweredDown, ! poweredDown);
    setMouseCursor(poweredDown ? juce::MouseCursor::NormalCursor : juce::MouseCursor::PointingHandCursor);
    repaint();
}

void MonoStereoSwitch::buttonStateChanged()
{
    const float target = getToggleState() ? 1.0f : 0.0f;
    if (std::abs(target - thumbTarget) < 1.0e-6f)
        return;

    thumbTarget = target;
    lastFrameMs = juce::Time::getMillisecondCounter();
    if (! isTimerRunning())
        startTimerHz(60);
}

void MonoStereoSwitch::timerCallback()
{
    const auto now = juce::Time::getMillisecondCounter();
    const float dtMs = juce::jlimit(1.0f, 100.0f, (float) (now - lastFrameMs));
    lastFrameMs = now;

    const float step = dtMs / travelMs;
    const float from = thumbPosition;
    const float remaining = thumbTarget - from;

    if (std::abs(remaining) < 0.001f)
    {
        thumbPosition = thumbTarget;
        stopTimer();
        repaint();
        return;
    }

    // Advance a linear progress variable and shape it, rather than easing the position itself -
    // easing an already-eased value would flatten the overshoot the spring curve exists for.
    const float linear = juce::jlimit(0.0f, 1.0f, std::abs(from - (1.0f - thumbTarget)) + step);
    const float shaped = springEase(linear);
    thumbPosition = thumbTarget > 0.5f ? shaped : 1.0f - shaped;
    repaint();
}

void MonoStereoSwitch::paintButton(juce::Graphics& g, bool shouldDrawHighlighted, bool)
{
    const auto bounds = getLocalBounds().toFloat();
    const float dim = poweredDown ? Layout::powerDownOpacity : 1.0f;

    // Track.
    {
        juce::ColourGradient gradient(trackTop.withMultipliedAlpha(dim), bounds.getX(), bounds.getY(),
                                      trackBottom.withMultipliedAlpha(dim), bounds.getX(), bounds.getBottom(),
                                      false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, trackCornerRadius);
    }

    // Inset shadow: 0 2px 6px rgba(0,0,0,.85), approximated by a soft inner stroke since JUCE has
    // no inset-shadow primitive.
    g.setColour(juce::Colours::black.withAlpha(0.85f * dim));
    g.drawRoundedRectangle(bounds.reduced(0.5f), trackCornerRadius, 1.0f);
    g.setColour(juce::Colours::black.withAlpha(0.35f * dim));
    g.drawRoundedRectangle(bounds.reduced(1.5f), trackCornerRadius - 1.0f, 1.5f);

    // Thumb - travels 24px between the two 4px-inset positions.
    const float thumbX = bounds.getX() + thumbInset;
    const float thumbY = bounds.getY() + thumbInset + thumbTravel * juce::jlimit(0.0f, 1.0f, thumbPosition);
    const juce::Rectangle<float> thumb(thumbX, thumbY, thumbDiameter, thumbDiameter);

    g.setColour(juce::Colours::black.withAlpha(0.45f * dim));
    g.fillEllipse(thumb.translated(0.0f, 1.0f));

    juce::ColourGradient thumbGradient(thumbTop.withMultipliedAlpha(dim), thumb.getX(), thumb.getY(),
                                        thumbBottom.withMultipliedAlpha(dim), thumb.getX(), thumb.getBottom(),
                                        false);
    g.setGradientFill(thumbGradient);
    g.fillEllipse(thumb);

    if (shouldDrawHighlighted && ! poweredDown)
    {
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.fillEllipse(thumb);
    }
}
