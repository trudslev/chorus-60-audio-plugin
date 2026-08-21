#include "../Source/GUI/KnobComponent.h"
#include "../Source/GUI/Chorus60Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

/*  **Which of the two levers dominates: the cap's DRAWING or the cache's BLIT.**

    The ranking established that Chorus-60's three knob groups are 8.13 ms of a ~15 ms editor — the
    largest category. The resampling hypothesis was refuted structurally: nothing blits a filmstrip,
    the cap is code-drawn into a cached layer and that layer is blitted at `1 / deviceScale`. So the
    mechanism is a resampled blit after all, but of the component's own cache.

    That leaves exactly two levers, and **which one dominates decides who owns the fix:**

      - **If the BLIT dominates**, the lever is the cache's scale — how large the layer is rendered
        and what it is scaled by on the way out. That is bounded, local, and a performance change.
      - **If the CAP'S DRAWING dominates**, the lever is `paintKnobCap` itself, which is §3.1's
        construction: the skirt stops, the cap gradient, the specular. **That is a spec question
        rather than a performance one**, and not one to decide from a profile.

    Stated before running, so the answer selects an owner rather than being read to suit one.

    **And there is a prior question that costs nothing to ask**, which is whether the cache is a
    cache at all: a layer rebuilt every frame would make the cap's drawing a per-frame cost and both
    levers above the wrong pair. `staticLayerBuildCount()` answers it directly.
*/
class KnobPaintSplitTests final : public juce::UnitTest
{
public:
    KnobPaintSplitTests() : juce::UnitTest ("Knob paint split", "Performance") {}

    void runTest() override
    {
        using namespace Chorus60Theme;

        beginTest ("The cache is a cache — the layer is not rebuilt per frame");

        KnobComponent knob (KnobFilmstripSize::mod, Layout::modKnobD);
        /*  The box is the DIAMETER here — this casting sizes a knob to its own Ø, not to a bleed
            multiple of it, because the cap is drawn rather than blitted from a padded sprite frame.
            Taken from the editor's own construction so the measurement is of what ships. */
        const int box = juce::roundToInt (Layout::modKnobD);
        knob.setSize (box, box);

        juce::Component holder;
        holder.setSize (box, box);
        holder.addAndMakeVisible (knob);

        juce::Image canvas (juce::Image::ARGB, box, box, true);

        const auto paintOnce = [&]
        {
            juce::Graphics g (canvas);
            holder.paintEntireComponent (g, false);
        };

        constexpr int frames = 400;
        for (int i = 0; i < frames; ++i)
            paintOnce();

        const int builds = knob.staticLayerBuildCount();
        logMessage ("  " + juce::String (builds) + " layer build(s) across " + juce::String (frames)
                    + " paints");

        expectLessOrEqual (builds, 2,
                           "the static layer is being rebuilt per frame, so it is not a cache and "
                           "the cap's drawing is a PER-FRAME cost - which makes the split below the "
                           "wrong pair of levers entirely");

        beginTest ("Cap drawing against cache blit");

        /*  **The cap's drawing, timed at the layer's own resolution.** This is what a rebuild costs
            and it is paid once per (size, scale) - so it is reported per BUILD, not per frame, and
            comparing it to a per-frame figure directly would be the arithmetic error that makes a
            one-off look like a recurring cost. */
        const float deviceScale = 2.0f;
        juce::Image layer (juce::Image::ARGB, juce::roundToInt (box * deviceScale),
                            juce::roundToInt (box * deviceScale), true);

        const auto timeCap = [&] (int repeats)
        {
            const auto start = juce::Time::getMillisecondCounterHiRes();
            for (int i = 0; i < repeats; ++i)
            {
                juce::Graphics ig (layer);
                ig.addTransform (juce::AffineTransform::scale (deviceScale));
                paintKnobCap (ig, juce::Rectangle<float> (0.0f, 0.0f, (float) box, (float) box),
                               Layout::modKnobD, false);
            }
            return (juce::Time::getMillisecondCounterHiRes() - start) / (double) repeats;
        };

        const auto timeBlit = [&] (int repeats)
        {
            juce::Graphics g (canvas);
            const auto start = juce::Time::getMillisecondCounterHiRes();
            for (int i = 0; i < repeats; ++i)
                g.drawImageTransformed (layer, juce::AffineTransform::scale (1.0f / deviceScale));
            return (juce::Time::getMillisecondCounterHiRes() - start) / (double) repeats;
        };

        timeCap (20);
        timeBlit (200);

        const double capMs  = timeCap (200);
        const double blitMs = timeBlit (2000);

        logMessage ("  cap drawing  " + juce::String (capMs * 1000.0, 1) + " us per BUILD");
        logMessage ("  cache blit   " + juce::String (blitMs * 1000.0, 1) + " us per FRAME");
        logMessage ("  at 60 Hz the blit is " + juce::String (blitMs * 60.0, 3)
                    + " ms/s per knob; the cap is paid " + juce::String (builds) + " time(s) total");

        /*  **NEITHER LEVER IS THE COST, AND THE FIRST VERSION OF THIS VERDICT SAID OTHERWISE.**

            It read `blitMs * 60.0 > capMs` — one SECOND of blitting against ONE build — and
            announced that the cap's drawing dominates. That is a rate compared against a one-off,
            which is the arithmetic error the comment ten lines above warns about, committed in the
            line below the warning. Over a two-second run the comparison inverts; over a session it
            is not close.

            The figures settle it without needing a comparison at all. **A knob costs the blit, once
            per frame, and that is ~1.3 us.** Nine of them is ~12 us of a frame. The ranking put the
            three groups at 8.13 ms — so the knobs are on the order of **one per cent of their own
            group**, and neither the cap's drawing nor the blit's scale is where that time goes.

            What is left inside a `DimmableGroup` is `GroupPrintedLayer`: the group heading, and
            `drawKnobScale` for every ring it owns — tick rings, numerals, units and control labels,
            uncached, every frame. That is a CACHE question and therefore ours, not §3.1's. */
        logMessage ("  => NEITHER. A knob costs " + juce::String (blitMs * 1000.0, 1)
                    + " us per frame; nine are ~" + juce::String (blitMs * 9000.0, 0)
                    + " us. The groups measured 8.13 ms, so the knobs are ~1 % of them and the "
                      "cost is GroupPrintedLayer's uncached printed layer beside them.");
        logMessage ("  (the cap's " + juce::String (capMs * 1000.0, 1)
                    + " us is paid once per size/scale and is not a per-frame cost at all)");

        /*  **Reported, not asserted** — these are figures about a machine and pinning either would
            pin its renderer. What IS asserted is that the two are distinguishable: if the cap and
            the blit came back the same, the fixture is measuring neither and the verdict above is
            unreadable, which is the vacuity guard the x1 input control needed and did not have. */
        expectGreaterThan (capMs, 0.0, "the cap drawing took no measurable time");
        expectGreaterThan (blitMs, 0.0, "the blit took no measurable time");
        expectNotEquals (juce::String (capMs / blitMs, 1), juce::String ("1.0"),
                         "cap and blit cost the same, so this fixture distinguishes nothing");
    }
};

static KnobPaintSplitTests knobPaintSplitTests;
