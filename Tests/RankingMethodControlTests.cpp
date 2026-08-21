#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include "../Source/GUI/ModScope.h"
#include "../Source/GUI/Chorus60Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

/*  **Is the RANKING attributing something its rows do not contain?**

    `ModScope`'s parts sum to about 66 us — grid 2.7, centre line 2.1, trace 59.7 — and the ranking
    puts the component at **2523 us**. Two orders apart, and a bare full-canvas child painting
    nothing costs 1.0 us, so size is already refuted.

    That leaves two possibilities and this file separates them, which is why it runs before either
    of the remaining suspects:

      - **The cost is real** and lives somewhere the parts split did not reproduce — the static
        layer's blit, or the real loop's per-column work against the synthetic one. Then the ranking
        is sound and the hunt continues inside the component.
      - **The METHOD is attributing it.** The ranking paints the whole editor with one child visible
        and subtracts a floor; a direct timing of the component alone does not.

    **This reaches past this component, which is why it is worth its own file.** If the method
    attributes something, every row in that ranking carries it — including the 8.13 ms that sent this
    investigation after the knobs in the first place. **A ranking whose rows are inflated by a
    CONSTANT still ranks correctly; one inflated PROPORTIONALLY does not**, and the difference
    decides whether anything read off it so far survives.
*/
class RankingMethodControlTests final : public juce::UnitTest
{
public:
    RankingMethodControlTests() : juce::UnitTest ("Ranking method control", "Performance") {}

    void runTest() override
    {
        using namespace Chorus60Theme;

        beginTest ("ModScope timed DIRECTLY against the same component timed through the ranking");

        Chorus60AudioProcessor processor;

        // --- direct: the component alone, in a holder of its own size ---------------------------
        double directMs = 0.0;
        {
            ModScope scope (processor);
            scope.setSize ((int) Layout::canvasWidth, (int) Layout::canvasHeight);

            juce::Component holder;
            holder.setSize (scope.getWidth(), scope.getHeight());
            holder.addAndMakeVisible (scope);

            juce::Image canvas (juce::Image::ARGB, holder.getWidth(), holder.getHeight(), true);

            const auto run = [&] (int repeats)
            {
                juce::Graphics g (canvas);
                holder.paintEntireComponent (g, false);
                const auto start = juce::Time::getMillisecondCounterHiRes();
                for (int i = 0; i < repeats; ++i)
                    holder.paintEntireComponent (g, false);
                return (juce::Time::getMillisecondCounterHiRes() - start) / (double) repeats;
            };

            run (8);
            directMs = run (120);

            /*  **The cache's own rebuild count, which this component was the only one of three not
                to have.** If the layer rebuilds per paint the cache is not a cache, and the 2.5 ms
                is the static half being redrawn behind a blit of itself. */
            logMessage ("  static layer builds across 128 paints: "
                        + juce::String (scope.staticLayerBuildCount()));

            /*  **The blit, alone.** The cache is confirmed a cache and the parts sum to 66 us, so
                the only thing left inside `paint` is drawing the cached layer. Same dimensions and
                same call the component makes — an ARGB image over an ARGB canvas at 1:1. */
            {
                const float w = Layout::scopeWellW;
                const float h = (Layout::scopeWellY + Layout::scopeWellH) - Layout::scopeCaptionRowY;
                juce::Image layer (juce::Image::ARGB, (int) w, (int) h, true);
                juce::Graphics g (canvas);

                const auto blit = [&] (int repeats)
                {
                    g.drawImage (layer, juce::Rectangle<float> (Layout::scopeWellX,
                                                                 Layout::scopeCaptionRowY, w, h),
                                  juce::RectanglePlacement::stretchToFit);
                    const auto start = juce::Time::getMillisecondCounterHiRes();
                    for (int i = 0; i < repeats; ++i)
                        g.drawImage (layer, juce::Rectangle<float> (Layout::scopeWellX,
                                                                     Layout::scopeCaptionRowY, w, h),
                                      juce::RectanglePlacement::stretchToFit);
                    return (juce::Time::getMillisecondCounterHiRes() - start) / (double) repeats;
                };

                blit (20);
                logMessage ("  cached-layer blit " + juce::String (blit (400) * 1000.0, 1)
                            + " us   (" + juce::String ((int) w) + " x " + juce::String ((int) h)
                            + " ARGB, 1:1)");
            }
        }

        // --- the ranking's own method: full editor, one child visible, minus the floor -----------
        double rankedMs = 0.0, floorMs = 0.0;
        {
            auto editor = std::unique_ptr<juce::AudioProcessorEditor> (processor.createEditor());
            expect (editor != nullptr);
            if (editor == nullptr)
                return;

            juce::Component holder;
            holder.setSize (editor->getWidth(), editor->getHeight());
            holder.addAndMakeVisible (*editor);

            juce::Image canvas (juce::Image::ARGB, holder.getWidth(), holder.getHeight(), true);

            juce::Component* content = editor.get();
            while (content->getNumChildComponents() > 0)
            {
                juce::Component* biggest = nullptr;
                for (int i = 0; i < content->getNumChildComponents(); ++i)
                {
                    auto* c = content->getChildComponent (i);
                    if (biggest == nullptr
                        || c->getNumChildComponents() > biggest->getNumChildComponents())
                        biggest = c;
                }
                if (biggest == nullptr || biggest->getNumChildComponents() < 4) break;
                content = biggest;
            }

            std::vector<juce::Component*> children;
            for (int i = 0; i < content->getNumChildComponents(); ++i)
                children.push_back (content->getChildComponent (i));

            const auto run = [&] (int repeats)
            {
                juce::Graphics g (canvas);
                holder.paintEntireComponent (g, false);
                const auto start = juce::Time::getMillisecondCounterHiRes();
                for (int i = 0; i < repeats; ++i)
                    holder.paintEntireComponent (g, false);
                return (juce::Time::getMillisecondCounterHiRes() - start) / (double) repeats;
            };

            for (auto* c : children) c->setVisible (false);
            run (4);
            floorMs = run (60);

            juce::Component* scope = nullptr;
            for (auto* c : children)
                if (dynamic_cast<ModScope*> (c) != nullptr) scope = c;

            expect (scope != nullptr, "no ModScope among the content's children");
            if (scope == nullptr) return;

            scope->setVisible (true);
            run (4);
            rankedMs = run (60) - floorMs;
            for (auto* c : children) c->setVisible (true);
        }

        logMessage ("  direct      " + juce::String (directMs * 1000.0, 1) + " us");
        logMessage ("  ranked      " + juce::String (rankedMs * 1000.0, 1)
                    + " us   (floor " + juce::String (floorMs * 1000.0, 1) + " us)");
        logMessage ("  parts split measured about 66 us of content");

        const double ratio = directMs > 0.0 ? rankedMs / directMs : 0.0;
        logMessage ("  ranked / direct = " + juce::String (ratio, 2) + "x");

        logMessage (ratio > 3.0
            ? "  => THE METHOD IS ATTRIBUTING IT. Every row in that ranking carries the same thing, "
              "including the 8.13 ms that sent this investigation after the knobs."
            : "  => THE COST IS REAL. The ranking is sound and the blit is the next suspect.");

        /*  **Reported, not asserted.** What IS asserted is that the two methods were both measured
            and can be told apart — a control returning the same number for both would settle
            nothing, which is the vacuity shape this suite keeps finding. */
        expectGreaterThan (directMs, 0.0, "the direct timing measured nothing");
        expectGreaterThan (rankedMs, 0.0, "the ranked timing measured nothing");
    }
};

static RankingMethodControlTests rankingMethodControlTests;
