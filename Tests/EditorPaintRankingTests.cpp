#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include <juce_audio_processors/juce_audio_processors.h>

/*  **Rank the children before assuming which one is the answer.**

    Chorus-60's editor-open figure is ~37-39 % of one core against Elmer's 5.5 % — a ~7x spread
    between two panels drawn to the same design system, which says something about construction
    rather than content. The hypothesis already on the table is its knob filmstrips: they are 1x
    sheets, so its knobs are upscaled at the default UI scale, and a blit is nearly free at 1:1 and
    10-20x that when resampled.

    **That hypothesis is not tested here, and deliberately so.** It was formed before measuring, it
    has a mechanism and a predicted magnitude, and the ranking costs the same whether it confirms it
    or names something else. So this measures first and the ranking either puts the knobs at the top
    or does not.

    **Each child is timed AS A CHILD, through the parent.** `paintEntireComponent` does not consult a
    component's image cache — it is what the cache calls to FILL itself. The cache is read one level
    up, in `paintWithinParentContext`, which runs only when a parent paints a child. Timing a child
    directly measures uncached paint and reports it as cached: Reflect-84's large knob came back
    4516 us "cached" against 4513 us uncached before that was caught.

    So every measurement below paints the EDITOR, with exactly one child visible, and subtracts the
    cost of painting it with none visible. Every child that is measured goes through
    `paintWithinParentContext`, which is the path a host uses.
*/
class EditorPaintRankingTests final : public juce::UnitTest
{
public:
    EditorPaintRankingTests() : juce::UnitTest ("Editor paint ranking", "Performance") {}

    void runTest() override
    {
        beginTest ("Which children cost what, painted as children");

        Chorus60AudioProcessor processor;
        auto editor = std::unique_ptr<juce::AudioProcessorEditor> (processor.createEditor());
        expect (editor != nullptr);
        if (editor == nullptr)
            return;

        juce::Component holder;
        holder.setSize (editor->getWidth(), editor->getHeight());
        holder.addAndMakeVisible (*editor);

        juce::Image frame (juce::Image::ARGB, juce::jmax (1, holder.getWidth()),
                            juce::jmax (1, holder.getHeight()), true);

        /*  **Descend to the component that actually holds the panel, not to the editor's own
            children.** The first version took the editor's children when it had exactly one, and
            this editor has three — so it ranked `Chorus60EditorContent` at 17.055 ms against a
            0.116 ms background and a 0.009 ms resizer, which is one row and answers nothing.

            The container is found by descending while a single child dominates the child count,
            which is a property of the tree rather than a name — the same reason `ringsInBox`
            partitions by containment instead of by a list of names. */
        juce::Component* content = editor.get();

        while (content->getNumChildComponents() > 0)
        {
            juce::Component* biggest = nullptr;
            for (int i = 0; i < content->getNumChildComponents(); ++i)
            {
                auto* c = content->getChildComponent (i);
                if (biggest == nullptr || c->getNumChildComponents() > biggest->getNumChildComponents())
                    biggest = c;
            }

            if (biggest == nullptr || biggest->getNumChildComponents() < 4)
                break;

            content = biggest;
        }

        logMessage ("  ranking the children of " + juce::String (typeid (*content).name())
                    + ", " + juce::String (content->getNumChildComponents()) + " of them");

        std::vector<juce::Component*> children;
        for (int i = 0; i < content->getNumChildComponents(); ++i)
            children.push_back (content->getChildComponent (i));

        expectGreaterThan ((int) children.size(), 1,
                           "only one child - the ranking would report one row and answer nothing");

        const auto paintAll = [&] (int repeats)
        {
            const auto start = juce::Time::getMillisecondCounterHiRes();
            for (int i = 0; i < repeats; ++i)
            {
                juce::Graphics g (frame);
                holder.paintEntireComponent (g, false);
            }
            return (juce::Time::getMillisecondCounterHiRes() - start) / (double) repeats;
        };

        constexpr int repeats = 24;

        std::vector<bool> wasVisible;
        for (auto* c : children)
            wasVisible.push_back (c->isVisible());

        // The floor: the editor with every child hidden. Whatever the editor paints itself - the
        // fascia, the plate, anything drawn in its own paint() - is in here and is not attributable
        // to any child, so subtracting it is what makes the rows comparable.
        for (auto* c : children)
            c->setVisible (false);

        paintAll (4);                       // warm every cache before anything is timed
        const double floorMs = paintAll (repeats);

        std::vector<std::pair<double, juce::String>> ranked;

        for (size_t i = 0; i < children.size(); ++i)
        {
            children[i]->setVisible (true);
            paintAll (2);
            const double withMs = paintAll (repeats);
            children[i]->setVisible (false);

            const auto name = children[i]->getName().isNotEmpty()
                                ? children[i]->getName()
                                : juce::String (typeid (*children[i]).name());

            ranked.emplace_back (withMs - floorMs,
                                 name + "  " + children[i]->getBounds().toString());
        }

        for (size_t i = 0; i < children.size(); ++i)
            children[i]->setVisible (wasVisible[i]);

        std::sort (ranked.begin(), ranked.end(),
                   [] (const auto& a, const auto& b) { return a.first > b.first; });

        const double allMs = paintAll (repeats);

        logMessage ("  editor with no children: " + juce::String (floorMs, 3) + " ms");
        logMessage ("  editor with all children: " + juce::String (allMs, 3) + " ms");
        logMessage ("  --- children, most expensive first ---");

        double summed = 0.0;
        for (const auto& [ms, name] : ranked)
        {
            summed += ms;
            logMessage ("  " + juce::String (ms, 3) + " ms   " + name);
        }

        logMessage ("  sum of children: " + juce::String (summed, 3) + " ms against "
                    + juce::String (allMs - floorMs, 3) + " ms measured together");

        /*  **A weak assertion on purpose.** This is a ranking, not a property: the figures are the
            output and pinning any of them would be pinning a machine's performance. What IS asserted
            is that the decomposition adds up — if the parts do not approximately account for the
            whole, the attribution is wrong and the ranking above means nothing, which is the same
            arithmetic check that caught a 2.00x generation sum being a multiplication table. */
        expectGreaterThan (allMs, floorMs, "adding every child cost nothing measurable");
        expectGreaterThan (summed, (allMs - floorMs) * 0.5,
                           "the children do not account for half of what they cost together, so the "
                           "attribution is wrong and the ranking cannot be read");
    }
};

static EditorPaintRankingTests editorPaintRankingTests;
