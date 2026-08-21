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
        /*  **Every comparison here lays the scope's own glass first.** An earlier version diffed
            ARGB images with a TRANSPARENT ground, so it measured alpha-channel differences in an
            unrendered image — and a viewer composites such an image on white, where a red glow is a
            different object. It reported 31.76/255 where the composited answer is 3.33. The figure
            was the instrument, not the subject, and the conclusion drawn from it was retracted. */
        const auto glassOnly = [&] (juce::Graphics& g)
        {
            juce::ColourGradient glass (Colour::scopeBgTop, w * 0.5f, 0.0f,
                                         Colour::scopeBgBottom, w * 0.5f, h, false);
            g.setGradientFill (glass);
            g.fillRect (0.0f, 0.0f, w, h);
        };

        const auto visualDelta = [&] (float scale)
        {
            juce::Image full (juce::Image::ARGB, (int) w, (int) h, true);
            { juce::Graphics fg (full); glassOnly (fg); renderGlow (fg, 1.0f); }

            juce::Image small (juce::Image::ARGB, juce::roundToInt (w * scale),
                                juce::roundToInt (h * scale), true);
            { juce::Graphics sg (small); renderGlow (sg, scale); }

            juce::Image up (juce::Image::ARGB, (int) w, (int) h, true);
            {
                juce::Graphics ug (up);
                glassOnly (ug);
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

        /*  **The visual price of what was actually SHIPPED, which is not the split's figure.**

            The 33/255 mean above was measured with the whole glow resampled, core stroke included —
            and its 255/255 worst case WAS that core stroke, whose hard edge is maximally wrong when
            scaled. The applied change resamples only the two blurs and draws the 7 px band, the
            3 px core and the head dot at full resolution.

            So the number that matters is this one: the same construction, blurs at half against
            blurs at full, with everything sharp identical in both. */
        {
            const auto p = buildTrace (1.0f);

            juce::Path glowOutline, coreOutline;
            juce::PathStrokeType (7.0f, juce::PathStrokeType::mitered,
                                   juce::PathStrokeType::butt).createStrokedPath (glowOutline, p);
            juce::PathStrokeType (3.0f, juce::PathStrokeType::mitered,
                                   juce::PathStrokeType::butt).createStrokedPath (coreOutline, p);

            const auto sharpPart = [&] (juce::Graphics& g)
            {
                g.setColour (Colour::chorusAccent.withAlpha (0.45f));
                g.fillPath (glowOutline);
                g.setColour (Colour::chorusAccent);
                g.strokePath (p, juce::PathStrokeType (3.0f, juce::PathStrokeType::mitered,
                                                        juce::PathStrokeType::butt));
            };

            /*  **Both renders sit on the scope's own glass, not on transparency.** The first
                version wrote ARGB images with a transparent ground; a viewer composites those on
                WHITE, and a red glow over white is a different object from a red glow over
                `#0B0F11 -> #050708`. The figures were right and the picture was misattributing
                them — the instrument-versus-subject error, arriving in a presentation. */
            const auto layGlass = [&] (juce::Graphics& g)
            {
                juce::ColourGradient glass (Colour::scopeBgTop, w * 0.5f, 0.0f,
                                             Colour::scopeBgBottom, w * 0.5f, h, false);
                g.setGradientFill (glass);
                g.fillRect (0.0f, 0.0f, w, h);
            };

            juce::Image before (juce::Image::ARGB, (int) w, (int) h, true);
            {
                juce::Graphics g (before);
                layGlass (g);
                juce::DropShadow (Colour::chorusAccent.withAlpha (0.80f), 20, {0, 0}).drawForPath (g, glowOutline);
                juce::DropShadow (Colour::chorusAccent.withAlpha (0.70f), 10, {0, 0}).drawForPath (g, coreOutline);
                sharpPart (g);
            }

            /*  **`after` now renders the PADDED construction that ships**, which is the whole
                point of this arm: the unpadded one put its raster boundary on the clip edge, and
                the chief designer found it in the diff — worst pixel at x = 1031 of 1035, four from
                the right, which is fray rather than glow. Both variants are built below so the
                move is shown rather than asserted. */
            const auto renderHalfRes = [&] (juce::Image& dest, float pad)
            {
                { juce::Graphics g (dest); layGlass (g); }

                constexpr float s = 0.5f;
                const juce::Rectangle<float> padded (-pad, -pad, w + pad * 2.0f, h + pad * 2.0f);

                juce::Image layer (juce::Image::ARGB,
                                    juce::roundToInt (padded.getWidth() * s),
                                    juce::roundToInt (padded.getHeight() * s), true);
                {
                    juce::Graphics bg (layer);
                    const auto toLayer = juce::AffineTransform::translation (-padded.getX(), -padded.getY())
                                             .scaled (s, s);
                    auto gs = glowOutline; gs.applyTransform (toLayer);
                    auto cs = coreOutline; cs.applyTransform (toLayer);
                    juce::DropShadow (Colour::chorusAccent.withAlpha (0.80f),
                                       juce::roundToInt (20.0f * s), {0, 0}).drawForPath (bg, gs);
                    juce::DropShadow (Colour::chorusAccent.withAlpha (0.70f),
                                       juce::roundToInt (10.0f * s), {0, 0}).drawForPath (bg, cs);
                }

                juce::Graphics g (dest);
                g.setImageResamplingQuality (juce::Graphics::mediumResamplingQuality);
                g.drawImage (layer, padded, juce::RectanglePlacement::stretchToFit);
                sharpPart (g);
            };

            juce::Image unpadded (juce::Image::ARGB, (int) w, (int) h, true);
            renderHalfRes (unpadded, 0.0f);

            juce::Image after (juce::Image::ARGB, (int) w, (int) h, true);
            renderHalfRes (after, 27.0f);

            /*  Where the worst pixel IS, not just how big it is. The designer's caveat is entirely
                about location: 24/255 in the middle of a falloff is invisible, and 24/255 four
                pixels from the clip is a hard stop at the end of the trace. */
            const auto worstAt = [&] (const juce::Image& variant)
            {
                int wx = 0, wy = 0, wd = 0;
                for (int y = 0; y < (int) h; ++y)
                    for (int x = 0; x < (int) w; ++x)
                    {
                        const auto p = before.getPixelAt (x, y), q = variant.getPixelAt (x, y);
                        const int d = juce::jmax (std::abs ((int) p.getRed()   - (int) q.getRed()),
                                                   std::abs ((int) p.getGreen() - (int) q.getGreen()),
                                                   std::abs ((int) p.getBlue()  - (int) q.getBlue()));
                        if (d > wd) { wd = d; wx = x; wy = y; }
                    }
                return std::tuple<int,int,int> { wd, wx, wy };
            };

            {
                const auto [du, xu, yu] = worstAt (unpadded);
                const auto [dp, xp, yp] = worstAt (after);
                logMessage ("  --------");
                logMessage ("  unpadded raster: worst " + juce::String (du) + "/255 at x="
                            + juce::String (xu) + " of " + juce::String ((int) w)
                            + ", y=" + juce::String (yu)
                            + (xu < 8 || xu > (int) w - 8 ? "   <- AT THE CLIP EDGE" : ""));
                logMessage ("  padded raster:   worst " + juce::String (dp) + "/255 at x="
                            + juce::String (xp) + " of " + juce::String ((int) w)
                            + ", y=" + juce::String (yp)
                            + (xp < 8 || xp > (int) w - 8 ? "   <- STILL AT THE EDGE" : "   (away from both edges)"));
            }

            double sum = 0.0; int worst = 0, n = 0, over16 = 0;
            for (int y = 0; y < (int) h; ++y)
                for (int x = 0; x < (int) w; ++x)
                {
                    const auto a = before.getPixelAt (x, y), b = after.getPixelAt (x, y);
                    const int d = juce::jmax (std::abs ((int) a.getRed()   - (int) b.getRed()),
                                               std::abs ((int) a.getGreen() - (int) b.getGreen()),
                                               std::abs ((int) a.getBlue()  - (int) b.getBlue()),
                                               std::abs ((int) a.getAlpha() - (int) b.getAlpha()));
                    sum += d; worst = juce::jmax (worst, d); ++n;
                    if (d > 16) ++over16;
                }

            /*  **The two images the ask is judged from.** Written out rather than described,
                because the delta figures below are a summary and a glow is not a thing anyone should
                accept or reject from a summary. Everything sharp is identical in both; only the two
                blurs differ. */
            if (const auto dir = juce::File (juce::SystemStats::getEnvironmentVariable (
                                                  "NF_GLOW_IMAGE_DIR", {}));
                dir.isDirectory())
            {
                for (const auto& [img, name] : { std::pair { &before, "glow-current.png" },
                                                  std::pair { &after,  "glow-half-resolution.png" } })
                {
                    juce::PNGImageFormat png;
                    auto out = dir.getChildFile (name);
                    out.deleteFile();
                    if (auto stream = out.createOutputStream())
                        png.writeImageToStream (*img, *stream);
                }
                logMessage ("  wrote both glow renders to " + dir.getFullPathName());
            }

            logMessage ("  --------");
            logMessage ("  APPLIED — blurs at half, core and band at full:");
            logMessage ("    visual delta mean " + juce::String (sum / (double) n, 2)
                        + "/255, worst " + juce::String (worst) + "/255, "
                        + juce::String (100.0 * over16 / (double) n, 2)
                        + " % of pixels differ by more than 16");
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
