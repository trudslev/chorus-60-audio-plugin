#include "ImageSwitch.h"
#include <cmath>

namespace
{
    using namespace Chorus60Theme;

    // cubic-bezier(.3, 1.5, .5, 1) - the overshoot is the point: the thumb passes its target and
    // settles back, which is what makes a two-position switch read as sprung rather than as a value
    // fading between two states. Solved by bisection on x because a CSS timing function is
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

ImageSwitch::ImageSwitch() : juce::Button("ImageSwitch")
{
    setClickingTogglesState(true);
    setWantsKeyboardFocus(false);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

ImageSwitch::~ImageSwitch()
{
    stopTimer();
}

void ImageSwitch::setPoweredDown(bool shouldBePoweredDown)
{
    if (poweredDown == shouldBePoweredDown)
        return;

    poweredDown = shouldBePoweredDown;

    // Section 9: pointer interaction is disabled on all knobs and the switch while bypassed. The
    // switch keeps its position and stays where it is - the dimming is the group's multiply, not
    // anything this component does to itself.
    setInterceptsMouseClicks(! poweredDown, ! poweredDown);
    setMouseCursor(poweredDown ? juce::MouseCursor::NormalCursor : juce::MouseCursor::PointingHandCursor);
}

void ImageSwitch::buttonStateChanged()
{
    const float target = getToggleState() ? 1.0f : 0.0f;
    if (std::abs(target - thumbTarget) < 1.0e-6f)
        return;

    thumbTarget = target;
    lastFrameMs = juce::Time::getMillisecondCounter();
    if (! isTimerRunning())
        startTimerHz(60);
}

void ImageSwitch::timerCallback()
{
    const auto now = juce::Time::getMillisecondCounter();
    const float dtMs = juce::jlimit(1.0f, 100.0f, (float) (now - lastFrameMs));
    lastFrameMs = now;

    const float step = dtMs / Layout::switchTravelMs;
    const float from = thumbPosition;

    if (std::abs(thumbTarget - from) < 0.001f)
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

void ImageSwitch::paintButton(juce::Graphics& g, bool, bool)
{
    using namespace Chorus60Theme;

    // Component bounds are the track's own rect, so both sprites are placed relative to it. The
    // track is empty artwork - the thumb is a separate sprite drawn over it, which is what lets the
    // 34 px of travel actually happen rather than crossfading two composites.
    const auto bounds = getLocalBounds().toFloat();

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

    const auto& track = switchTrackImage();
    g.drawImage(track, (int) bounds.getX(), (int) bounds.getY(),
                 (int) bounds.getWidth(), (int) bounds.getHeight(),
                 0, 0, track.getWidth(), track.getHeight());

    // Section 7.2 puts the thumb at y 347 for STEREO and y 381 for MONO - 34 px apart - against a
    // track topped at 343, so the thumb's own inset within the track is 4 px.
    const float travel = Layout::switchThumbYMono - Layout::switchThumbYStereo;
    const float inset = Layout::switchThumbYStereo - Layout::switchTrackY;
    const float thumbX = bounds.getX() + (Layout::switchThumbX - Layout::switchTrackX);
    const float thumbY = bounds.getY() + inset + travel * juce::jlimit(0.0f, 1.0f, thumbPosition);

    const auto& thumb = switchThumbImage();
    g.drawImage(thumb, (int) std::round(thumbX), (int) std::round(thumbY),
                 (int) Layout::switchThumbD, (int) Layout::switchThumbD,
                 0, 0, thumb.getWidth(), thumb.getHeight());
}
