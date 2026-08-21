#include "../Source/GUI/Chorus60Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

/*  **ModScope's live half: the grid against the trace, measured before either is touched.**

    Its static half is cached and returned 27 %; what is left is 2.523 ms per paint at 60 Hz — the
    largest continuous cost on this panel. Two candidates inside it, and a mechanism already proposed
    for one: the scrolling grid could be a tile blitted at an offset.

    **That mechanism is held rather than acted on**, for the fifth time this week. The split either
    puts the grid at the top or names the trace, and it costs the same either way.

    **The pre-stated condition for the tile, answered by READING before this ran.** A scrolling grid
    cached as a tile only pays if the scroll offset is the sole thing changing; if its spacing or
    extent tracks a parameter, the tile rebuilds on any change to it and the win is smaller than the
    per-frame figure suggests — the same shape as keying `ProgramHeader`'s layer on the meter values.
    Here `gridSpacing = scopeWellW / scopeNumDivisions` and `pixelsPerFrame =
    scopeWellW / (scopeHistorySeconds * scopeFps)` are **all four constants**, and the loop varies
    only by `gridScrollPhase`. So the condition holds in its strongest form: a tile would never
    rebuild at all.

    Which makes the split the only open question — whether the thing that would never rebuild is
    also the thing that costs.
*/
class ScopeLiveHalfSplitTests final : public juce::UnitTest
{
public:
    ScopeLiveHalfSplitTests() : juce::UnitTest ("Scope live half split", "Performance") {}

    void runTest() override
    {
        using namespace Chorus60Theme;
        using namespace Chorus60Theme::Layout;

        beginTest ("Scrolling grid against trace, at the panel's own counts");

        const juce::Rectangle<float> inner (0.0f, 0.0f, scopeWellW - 4.0f, scopeWellH - 4.0f);
        juce::Image canvas (juce::Image::ARGB, (int) scopeWellW, (int) scopeWellH, true);

        const float gridSpacing = scopeWellW / (float) scopeNumDivisions;
        const float pixelsPerFrame = scopeWellW / (scopeHistorySeconds * scopeFps);
        const int columns = (int) std::ceil (inner.getWidth() / pixelsPerFrame) + 1;

        logMessage ("  grid: " + juce::String ((int) (inner.getWidth() / gridSpacing) + 1)
                    + " vertical lines at " + juce::String (gridSpacing, 1) + " px");
        logMessage ("  trace: " + juce::String (columns) + " columns at "
                    + juce::String (pixelsPerFrame, 2) + " px");

        const auto time = [&] (int repeats, auto&& body)
        {
            juce::Graphics g (canvas);
            body (g);
            const auto start = juce::Time::getMillisecondCounterHiRes();
            for (int i = 0; i < repeats; ++i)
                body (g);
            return (juce::Time::getMillisecondCounterHiRes() - start) / (double) repeats;
        };

        constexpr int n = 400;

        const double gridMs = time (n, [&] (juce::Graphics& g)
        {
            g.setColour (Colour::scopeGrid);
            for (float x = inner.getRight() - 3.0f; x >= inner.getX(); x -= gridSpacing)
                g.drawVerticalLine ((int) x, inner.getY(), inner.getBottom());
        });

        const double centreMs = time (n, [&] (juce::Graphics& g)
        {
            g.setColour (Colour::scopeCentreLine);
            g.drawHorizontalLine ((int) inner.getCentreY(), inner.getX(), inner.getRight());
        });

        /*  The trace is TWO things sharing one loop: a grey underlay drawn as one vertical line per
            column, and the red path stroked once. Timed apart, because "the trace" being the answer
            would still leave which half of it. */
        const double underlayMs = time (n, [&] (juce::Graphics& g)
        {
            g.setColour (Colour::scopeInputUnderlay);
            for (int c = 0; c < columns; ++c)
            {
                const float x = inner.getRight() - (float) c * pixelsPerFrame;
                const float y = inner.getCentreY() + 12.0f * std::sin ((float) c * 0.3f);
                g.drawVerticalLine ((int) x, y - 6.0f, y + 6.0f);
            }
        });

        const double pathMs = time (n, [&] (juce::Graphics& g)
        {
            juce::Path p;
            for (int c = 0; c < columns; ++c)
            {
                const float x = inner.getRight() - (float) c * pixelsPerFrame;
                const float y = inner.getCentreY() + 20.0f * std::sin ((float) c * 0.2f);
                if (c == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
            }
            g.setColour (Colour::chorusAccent);
            g.strokePath (p, juce::PathStrokeType (2.0f));
        });

        /*  **THE GLOW, which the first version of this split did not replicate at all.** §5 asks
            for a glow pass — a 7 px stroked outline under a **20 px** DropShadow — then a core pass
            with a **10 px** one. `DropShadow::drawForPath` renders the path into an image and
            box-blurs it at that radius; two of them, every frame, over a 1035 px wide trace.

            Its absence is why the parts did not add up: grid, centre line, underlay and a single
            stroke summed to 66 us against a component measuring 2428. */
        double glowMs = 0.0;
        {
            juce::Path p;
            for (int c = 0; c < columns; ++c)
            {
                const float x = inner.getRight() - (float) c * pixelsPerFrame;
                const float y = inner.getCentreY() + 20.0f * std::sin ((float) c * 0.2f);
                if (c == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
            }

            glowMs = time (40, [&] (juce::Graphics& g)
            {
                juce::Path glowOutline;
                juce::PathStrokeType (7.0f, juce::PathStrokeType::mitered,
                                       juce::PathStrokeType::butt).createStrokedPath (glowOutline, p);
                juce::DropShadow (Colour::chorusAccent.withAlpha (0.80f), 20, {0, 0})
                    .drawForPath (g, glowOutline);
                g.setColour (Colour::chorusAccent.withAlpha (0.45f));
                g.fillPath (glowOutline);

                juce::Path coreOutline;
                juce::PathStrokeType (3.0f, juce::PathStrokeType::mitered,
                                       juce::PathStrokeType::butt).createStrokedPath (coreOutline, p);
                juce::DropShadow (Colour::chorusAccent.withAlpha (0.70f), 10, {0, 0})
                    .drawForPath (g, coreOutline);
            });
        }

        const double traceMs = underlayMs + pathMs + glowMs;

        logMessage ("  --------");
        logMessage ("  grid          " + juce::String (gridMs * 1000.0, 1) + " us");
        logMessage ("  centre line   " + juce::String (centreMs * 1000.0, 1) + " us");
        logMessage ("  underlay      " + juce::String (underlayMs * 1000.0, 1) + " us");
        logMessage ("  trace path    " + juce::String (pathMs * 1000.0, 1) + " us");
        logMessage ("  GLOW          " + juce::String (glowMs * 1000.0, 1)
                    + " us   two DropShadow passes, 20 px and 10 px");
        logMessage ("  trace total   " + juce::String (traceMs * 1000.0, 1) + " us");
        logMessage (gridMs > traceMs
            ? "  => THE GRID DOMINATES. The tile pays, and its condition already holds: nothing but "
              "the offset varies, so it would never rebuild."
            : "  => THE TRACE DOMINATES. The tile would cache the cheap half — the mechanism is "
              "sound and aimed at the wrong object, which is this week's recurring shape.");

        /*  **THE PARTS DO NOT ACCOUNT FOR THE WHOLE, AND THAT IS THE REAL FINDING.** Grid, centre
            line, underlay and trace sum to about 64 us. `ModScope` measures **2523 us** per paint in
            the ranking. Two orders apart, so the live half is not the cost either — and neither was
            the static half, which is cached.

            The candidate is the component's SIZE rather than its content: `ModScope`'s bounds are
            the whole 1340 x 812 canvas, and a non-opaque child of that size makes its parent
            composite every pixel of it whether anything was drawn there or not. The arm below is the
            control — a bare `Component` that paints NOTHING, at the same bounds, through the same
            holder. If it costs what ModScope costs, the content was never the question. */
        beginTest ("The control: a full-canvas child that paints nothing");
        {
            struct Blank final : juce::Component { void paint (juce::Graphics&) override {} };

            Blank blank;
            blank.setSize ((int) canvasWidth, (int) canvasHeight);

            juce::Component holder;
            holder.setSize ((int) canvasWidth, (int) canvasHeight);
            holder.addAndMakeVisible (blank);

            juce::Image full (juce::Image::ARGB, (int) canvasWidth, (int) canvasHeight, true);

            const auto timeHolder = [&] (int repeats)
            {
                juce::Graphics g (full);
                holder.paintEntireComponent (g, false);
                const auto start = juce::Time::getMillisecondCounterHiRes();
                for (int i = 0; i < repeats; ++i)
                    holder.paintEntireComponent (g, false);
                return (juce::Time::getMillisecondCounterHiRes() - start) / (double) repeats;
            };

            const double blankMs = timeHolder (120);

            logMessage ("  a 1340 x 812 child painting NOTHING costs "
                        + juce::String (blankMs * 1000.0, 1) + " us");
            logMessage ("  ModScope's own content measured " + juce::String (traceMs * 1000.0
                        + gridMs * 1000.0 + centreMs * 1000.0, 1) + " us");
            logMessage (blankMs * 1000.0 > 500.0
                ? "  => SIZE, NOT CONTENT. A non-opaque full-canvas child is composited whole; the "
                  "lever is ModScope's BOUNDS, not its paint."
                : "  => not size. The blank child is cheap, so the cost is elsewhere in the paint.");

            expectGreaterThan (blankMs, 0.0);
        }

        expectGreaterThan (gridMs, 0.0);
        expectGreaterThan (traceMs, 0.0);
        expectNotEquals (juce::String (gridMs / traceMs, 1), juce::String ("1.0"),
                         "grid and trace cost the same, so this fixture distinguishes nothing");
    }
};

static ScopeLiveHalfSplitTests scopeLiveHalfSplitTests;
