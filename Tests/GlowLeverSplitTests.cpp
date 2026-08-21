#include "../Source/GUI/Chorus60Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

/*  **Two levers on a 4844 us glow, split before either is chosen — and one of them has a visible
    consequence the other does not.**

    `ModScope`'s cost is two `DropShadow` passes a frame: a 20 px blur on a 7 px stroked outline,
    then a 10 px blur on a 3 px one. §5 asks for the glow, so removing it is not on the table. How it
    is rendered is.

      - **Reduced resolution** — render the glow into a smaller image and scale up. A blur is
        low-frequency by definition, so this is the mechanism that fits what is being drawn. Its
        visible cost is **a slightly softer glow, on every frame**.
      - **Reduced update rate** — keep the glow from an earlier frame for N frames. Its cost is
        1/N by construction, no measurement needed. Its visible cost is **a glow that lags the trace
        it belongs to**.

    **PRE-STATED: if the two land close, prefer resolution.** A slightly softer glow is a property of
    the glow; a glow trailing its own trace is a defect, and the eye finds motion mismatches far more
    readily than it finds softness. So the update-rate lever has to win by a margin to be worth its
    consequence, not merely tie.

    **And the visual cost is measured rather than described.** Each reduced-resolution glow is
    compared against the full-resolution one pixel by pixel, so "slightly softer" is a figure.
*/
class GlowLeverSplitTests final : public juce::UnitTest
{
public:
    GlowLeverSplitTests() : juce::UnitTest ("Glow lever split", "Performance") {}

    void runTest() override
    {
        using namespace Chorus60Theme;
        using namespace Chorus60Theme::Layout;

        beginTest ("Reduced resolution against reduced update rate");

        const float w = scopeWellW - 4.0f, h = scopeWellH - 4.0f;
        const int columns = (int) std::ceil (w / (scopeWellW / (scopeHistorySeconds * scopeFps))) + 1;
        const float pxPerCol = w / (float) (columns - 1);

        const auto buildTrace = [&] (float scale)
        {
            juce::Path p;
            for (int c = 0; c < columns; ++c)
            {
                const float x = (float) c * pxPerCol * scale;
                const float y = (h * 0.5f + 20.0f * std::sin ((float) c * 0.2f)) * scale;
                if (c == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
            }
            return p;
        };

        /*  The glow exactly as `ModScope` draws it, at a given scale. At scale 1 this is the
            shipping cost; below 1 the outline widths and the blur radii scale with it, because a
            blur rendered at half size and doubled is a blur of twice the radius otherwise. */
        const auto renderGlow = [&] (juce::Graphics& g, float scale)
        {
            const auto p = buildTrace (scale);

            juce::Path glowOutline;
            juce::PathStrokeType (7.0f * scale, juce::PathStrokeType::mitered,
                                   juce::PathStrokeType::butt).createStrokedPath (glowOutline, p);
            juce::DropShadow (Colour::chorusAccent.withAlpha (0.80f),
                               juce::roundToInt (20.0f * scale), {0, 0}).drawForPath (g, glowOutline);
            g.setColour (Colour::chorusAccent.withAlpha (0.45f));
            g.fillPath (glowOutline);

            juce::Path coreOutline;
            juce::PathStrokeType (3.0f * scale, juce::PathStrokeType::mitered,
                                   juce::PathStrokeType::butt).createStrokedPath (coreOutline, p);
            juce::DropShadow (Colour::chorusAccent.withAlpha (0.70f),
                               juce::roundToInt (10.0f * scale), {0, 0}).drawForPath (g, coreOutline);
        };

        const auto timeAt = [&] (float scale, int repeats)
        {
            juce::Image layer (juce::Image::ARGB,
                                juce::jmax (1, juce::roundToInt (w * scale)),
                                juce::jmax (1, juce::roundToInt (h * scale)), true);
            juce::Image dest (juce::Image::ARGB, (int) w, (int) h, true);

            const auto once = [&]
            {
                layer.clear (layer.getBounds());
                { juce::Graphics lg (layer); renderGlow (lg, scale); }
                if (scale < 1.0f)
                {
                    juce::Graphics dg (dest);
                    dg.setImageResamplingQuality (juce::Graphics::mediumResamplingQuality);
                    dg.drawImage (layer, juce::Rectangle<float> (0.0f, 0.0f, w, h),
                                   juce::RectanglePlacement::stretchToFit);
                }
            };

            once();
            const auto start = juce::Time::getMillisecondCounterHiRes();
            for (int i = 0; i < repeats; ++i) once();
            return (juce::Time::getMillisecondCounterHiRes() - start) / (double) repeats;
        };

        /*  The visual cost, as a figure. Render the glow at `scale`, scale it back up, and compare
            it to the full-resolution one pixel by pixel — so "slightly softer" is measured rather
            than asserted. */
        const auto visualDelta = [&] (float scale)
        {
            juce::Image full (juce::Image::ARGB, (int) w, (int) h, true);
            { juce::Graphics fg (full); renderGlow (fg, 1.0f); }

            juce::Image small (juce::Image::ARGB, juce::roundToInt (w * scale),
                                juce::roundToInt (h * scale), true);
            { juce::Graphics sg (small); renderGlow (sg, scale); }

            juce::Image up (juce::Image::ARGB, (int) w, (int) h, true);
            {
                juce::Graphics ug (up);
                ug.setImageResamplingQuality (juce::Graphics::mediumResamplingQuality);
                ug.drawImage (small, juce::Rectangle<float> (0.0f, 0.0f, w, h),
                               juce::RectanglePlacement::stretchToFit);
            }

            double sum = 0.0; int worst = 0, n = 0;
            for (int y = 0; y < (int) h; y += 2)
                for (int x = 0; x < (int) w; x += 2)
                {
                    const auto a = full.getPixelAt (x, y), b = up.getPixelAt (x, y);
                    const int d = juce::jmax (std::abs ((int) a.getRed()   - (int) b.getRed()),
                                               std::abs ((int) a.getGreen() - (int) b.getGreen()),
                                               std::abs ((int) a.getBlue()  - (int) b.getBlue()),
                                               std::abs ((int) a.getAlpha() - (int) b.getAlpha()));
                    sum += d; worst = juce::jmax (worst, d); ++n;
                }
            return std::pair<double, int> { n > 0 ? sum / (double) n : 0.0, worst };
        };

        const double fullMs = timeAt (1.0f, 20);
        logMessage ("  full resolution      " + juce::String (fullMs * 1000.0, 1) + " us   (shipping)");

        for (const float scale : { 0.5f, 0.25f })
        {
            const double ms = timeAt (scale, 40);
            const auto [meanD, worstD] = visualDelta (scale);
            logMessage ("  at " + juce::String (scale, 2) + " resolution   "
                        + juce::String (ms * 1000.0, 1) + " us   "
                        + juce::String (fullMs / ms, 1) + "x cheaper   "
                        + "visual delta mean " + juce::String (meanD, 1) + "/255, worst "
                        + juce::String (worstD) + "/255");
        }

        /*  **A third lever, added because the measurement refuted the first.** Both named levers
            are poor: resolution is SLOWER at 0.5 and only 1.9x at 0.25 for a visibly different
            glow, and update rate buys its 1/N with a lag.

            The reason is the same for both: `juce::DropShadow` is a software box blur, and its cost
            does not fall the way its inputs do. So the option neither lever considered is not to
            blur at all — a glow is a few translucent strokes of increasing width, which is what a
            blurred stroke approximates and costs strokes rather than blurs. It is a rendering change
            and not a spec change: §5 asks for a glow, and this is one. */
        {
            const auto p = buildTrace (1.0f);
            juce::Image dest (juce::Image::ARGB, (int) w, (int) h, true);

            const auto stack = [&] (int repeats)
            {
                juce::Graphics g (dest);
                const auto once = [&]
                {
                    for (const auto& [width, alpha] : { std::pair {27.0f, 0.06f},
                                                         std::pair {19.0f, 0.10f},
                                                         std::pair {13.0f, 0.16f},
                                                         std::pair { 7.0f, 0.45f} })
                    {
                        g.setColour (Colour::chorusAccent.withAlpha (alpha));
                        g.strokePath (p, juce::PathStrokeType (width, juce::PathStrokeType::curved,
                                                                juce::PathStrokeType::rounded));
                    }
                    g.setColour (Colour::chorusAccent);
                    g.strokePath (p, juce::PathStrokeType (3.0f, juce::PathStrokeType::mitered,
                                                            juce::PathStrokeType::butt));
                };
                once();
                const auto start = juce::Time::getMillisecondCounterHiRes();
                for (int i = 0; i < repeats; ++i) once();
                return (juce::Time::getMillisecondCounterHiRes() - start) / (double) repeats;
            };

            const double stackMs = stack (200);
            logMessage ("  --------");
            logMessage ("  stroke stack, no blur  " + juce::String (stackMs * 1000.0, 1)
                        + " us   " + juce::String (fullMs / stackMs, 0)
                        + "x cheaper than the blur, every frame, no lag");
        }

        logMessage ("  --------");
        logMessage ("  update-rate lever is 1/N by construction and needs no measurement:");
        logMessage ("    every 2nd frame -> " + juce::String (fullMs * 500.0, 1)
                    + " us/frame, glow lags the trace by up to 16.7 ms");
        logMessage ("    every 4th frame -> " + juce::String (fullMs * 250.0, 1)
                    + " us/frame, lags by up to 50 ms");

        expectGreaterThan (fullMs, 0.0);
    }
};

static GlowLeverSplitTests glowLeverSplitTests;
