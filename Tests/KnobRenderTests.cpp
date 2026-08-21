#include "../Source/GUI/KnobComponent.h"
#include "../Source/GUI/Chorus60Theme.h"

#include <juce_gui_basics/juce_gui_basics.h>

/**
    The code-drawn knob, and specifically its cache.

    **A cache that rebuilds every frame is indistinguishable from a cache that works, by looking at
    the panel.** Both draw the right knob. The difference is only in how much work a drag costs, and
    a knob repaints on every frame of one — which is exactly the trap `setBufferedToImage` sets on a
    Slider, and the reason `staticLayerBuildCount` exists as a seam rather than the cache being
    considered self-evident.

    So every arm here is about **how many times the static layer was built**, not about what it
    looks like.
*/
class KnobRenderTests final : public juce::UnitTest
{
public:
    KnobRenderTests() : juce::UnitTest ("Knob rendering", "GUI") {}

    /** Renders the knob into an offscreen image the way a repaint would. */
    static void render (KnobComponent& knob, juce::Image& target)
    {
        juce::Graphics g { target };
        knob.paint (g);
    }

    void runTest() override
    {
        using namespace Chorus60Theme;

        beginTest ("The static layer is built once and reused across value changes");
        {
            KnobComponent knob { KnobFilmstripSize::mod, Layout::modKnobD };
            knob.setBounds (0, 0, (int) Layout::modKnobD, (int) Layout::modKnobD);
            knob.setRange (0.0, 1.0);

            juce::Image target { juce::Image::ARGB, (int) Layout::modKnobD,
                                 (int) Layout::modKnobD, true };

            render (knob, target);
            const int afterFirst = knob.staticLayerBuildCount();
            expectEquals (afterFirst, 1, "the first paint did not build a static layer at all");

            // Thirty frames of a drag: the pointer moves every time, the cap does not.
            for (int i = 0; i < 30; ++i)
            {
                knob.setValue ((double) i / 29.0, juce::dontSendNotification);
                render (knob, target);
            }

            logMessage ("  30 value changes -> " + juce::String (knob.staticLayerBuildCount())
                        + " static layer build(s)");
            expectEquals (knob.staticLayerBuildCount(), 1,
                          "the static layer rebuilt while only the pointer moved — this is a cache "
                          "in name only, and a drag pays for it every frame");
        }

        beginTest ("SHOWN ABLE TO FAIL: the counter moves when the layer genuinely must be rebuilt");
        {
            /*  The arm above asserts a count of 1 after thirty renders, and a counter that never
                incremented would satisfy it just as well as a working cache. So this drives the two
                things that MUST invalidate the layer and asserts the count rises — which is the
                same shape as core's allocation sentinel proving it can catch as well as clear. */
            KnobComponent knob { KnobFilmstripSize::global, Layout::globalKnobD };
            knob.setBounds (0, 0, (int) Layout::globalKnobD, (int) Layout::globalKnobD);

            juce::Image target { juce::Image::ARGB, (int) Layout::globalKnobD,
                                 (int) Layout::globalKnobD, true };
            render (knob, target);
            const int baseline = knob.staticLayerBuildCount();

            // 1 — the OFF state. §7.2 turns the specular off, and the specular is static.
            knob.setPoweredDown (true);
            render (knob, target);
            expectGreaterThan (knob.staticLayerBuildCount(), baseline,
                               "powering down did not rebuild the layer, so the specular is still "
                               "showing on a bypassed knob");

            const int afterDim = knob.staticLayerBuildCount();

            // 2 — a size change, which is what a device-scale change looks like to the cache.
            juce::Image bigger { juce::Image::ARGB, (int) Layout::globalKnobD * 2,
                                 (int) Layout::globalKnobD * 2, true };
            knob.setBounds (0, 0, (int) Layout::globalKnobD * 2, (int) Layout::globalKnobD * 2);
            render (knob, bigger);
            expectGreaterThan (knob.staticLayerBuildCount(), afterDim,
                               "a resized knob reused a layer drawn at the old size, which is what "
                               "a cache keyed on the wrong thing does");
        }

        beginTest ("The pointer follows the parameter's own travel, not a linear value");
        {
            /*  RATE is skewed 0.35, and the pointer has to land on marks that `drawKnobScale`
                places from the same range. The Slider's `valueToProportionOfLength` is what carries
                that, so this asserts the knob reports TRAVEL rather than value — the property the
                printed scale depends on. */
            KnobComponent knob { KnobFilmstripSize::mod, Layout::modKnobD };
            knob.setNormalisableRange ({ 0.05, 16.0, 0.0, 0.35 });

            knob.setValue (0.5, juce::dontSendNotification);
            const float atHalfHz = knob.getDrawnProportion();

            logMessage ("  RATE 0.5 Hz sits at travel " + juce::String (atHalfHz, 6));

            expect (atHalfHz > 0.20f && atHalfHz < 0.36f,
                    "0.5 Hz on a 0.05-16 range skewed 0.35 should sit near §3.2's 0.286852, and a "
                    "LINEAR reading would put it at 0.028 — which is what a pointer ignoring the "
                    "skew would draw");
        }
    }
};

static KnobRenderTests knobRenderTests;
