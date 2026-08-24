#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include "../Source/GUI/Chorus60Theme.h"

#include <nf/AboutPart.h>
#include <nf/testing/ComponentRender.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    `ABOUT-PART.md` against the real editor.

    **Why the assertions are here rather than in a capture.** Driving the standalone re-prompts for
    microphone permission on every rebuild, so there is no window until a human answers a dialog;
    and a capture's resolution is the display's, so the same figure changes instrument between
    machines. `nf::testing::renderComponent` runs the real paint path with neither problem. What it
    cannot see is anything the window server owns — focus, the cursor, z-order against other
    windows — so this file asserts geometry and writes an image for a person to look at, and does
    not claim to have tested the interaction.

    **The PNGs are the point of writing them at all.** A layer that is drawn, correct and invisible
    passes every assertion here: the About tab was under the panel for one build on Reflect-84,
    because JUCE paints children in the order they are added and the tab was registered beside its
    construction. Nothing in an assertion caught that. The image did.
*/
class AboutPartTests final : public juce::UnitTest
{
public:
    AboutPartTests() : juce::UnitTest ("About part", "GUI") {}

    void runTest() override
    {
        using namespace Chorus60Theme;

        beginTest ("the tab and box sit where ABOUT-PART states, and the part renders");

        Chorus60AudioProcessor processor;
        auto* editor = dynamic_cast<Chorus60AudioProcessorEditor*> (processor.createEditor());
        expect (editor != nullptr, "no editor");
        std::unique_ptr<juce::AudioProcessorEditor> owner { editor };

        // The content is what carries the part; the editor is the transform around it.
        juce::Component* content = nullptr;
        for (int i = 0; i < editor->getNumChildComponents(); ++i)
            if (editor->getChildComponent (i)->getWidth() == (int) Layout::canvasWidth)
                content = editor->getChildComponent (i);
        expect (content != nullptr, "no content component at the canvas width");

        nf::AboutTab* tab = nullptr;
        nf::AboutBox* box = nullptr;
        nf::AboutWordmarkHit* mark = nullptr;
        for (int i = 0; i < content->getNumChildComponents(); ++i)
        {
            auto* c = content->getChildComponent (i);
            if (auto* t = dynamic_cast<nf::AboutTab*> (c)) tab = t;
            if (auto* b = dynamic_cast<nf::AboutBox*> (c)) box = b;
            if (auto* w = dynamic_cast<nf::AboutWordmarkHit*> (c)) mark = w;
        }
        expect (tab != nullptr, "no AboutTab on the panel");
        expect (box != nullptr, "no AboutBox on the panel");
        expect (mark != nullptr, "no AboutWordmarkHit — §2a's PRIMARY affordance");

        // §2a: the hit box is the nameplate ZONE, not the letterforms — one figure for six
        // castings, and immune to the artwork-versus-text difference that ruled the wordmark out.
        expect (mark->getBounds() == nf::HeaderGeometry::nameplate(),
                "the wordmark hit box is " + mark->getBounds().toString()
                    + ", §2a says the nameplate zone " + nf::HeaderGeometry::nameplate().toString());
        expect (mark->getWidth() == 303 && mark->getHeight() == 84, "§2a states 303 x 84");

        const int canvasH = (int) Layout::canvasHeight;

        // §4's minimum, checked per casting because the law is the part's and the canvas is not.
        expect (canvasH >= nf::AboutGeometry::minCanvasH,
                "canvas " + juce::String (canvasH) + " is under the part's 620 minimum");

        // §2: right edge 1302, bottom canvasH - 20, height 24. Width is shrink-to-fit, so only the
        // right edge is fixed — asserting a width here would pin a figure the part leaves open.
        expect (tab->getRight() == nf::AboutGeometry::tabRight,
                "tab right edge " + juce::String (tab->getRight()) + ", §2 says 1302");
        expect (tab->getBottom() == canvasH - nf::AboutGeometry::tabBottomInset,
                "tab bottom " + juce::String (tab->getBottom()));
        expect (tab->getHeight() == nf::AboutGeometry::tabH, "tab height");

        // §4: 880 x 540, x = 230, y = (canvasH - 540) / 2.
        // §4's law is frame-local; this casting has no rack ears, so its frame origin is 0.
        const auto expected = nf::AboutGeometry::boxFor (canvasH, 0);
        expect (box->getBounds() == juce::Rectangle<int> (0, 0, (int) Layout::canvasWidth, canvasH),
                "the box's component spans the canvas — its veil is the dismissal target");
        logMessage ("  box law gives " + expected.toString() + " inside a "
                    + juce::String ((int) Layout::canvasWidth) + " x " + juce::String (canvasH) + " canvas");

        // §4's budget. 45 is the longest slug in the suite and the row must not wrap.
        expect (nf::AboutBox::repositoryFits ("github.com/trudslev/chorus-60-audio-plugin"),
                "the repository slug is over §4's 45-character budget");

        //== the images ==========================================================================
        const auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                             .getChildFile ("nf-about");

        expect (nf::testing::writeComponentPng (*content, dir.getChildFile ("chorus-60-rest.png")),
                "could not write the resting panel");

        // §6's affordance, exercised through the tab's own callback rather than a synthetic click:
        // there is no windowed peer here for an event to arrive through.
        expect (tab->onClick != nullptr, "the tab opens nothing");
        expect (mark->onClick != nullptr, "the wordmark opens nothing");

        // §2a: BOTH affordances open the same box. The wordmark is primary and is checked first.
        mark->onClick();
        expect (box->isVisible(), "the wordmark did not open the box");
        box->close();
        expect (! box->isVisible(), "close() did not hide the box");

        tab->onClick();
        expect (box->isVisible(), "the tab did not open the box");

        expect (nf::testing::writeComponentPng (*content, dir.getChildFile ("chorus-60-about.png")),
                "could not write the open box");

        // §6: all three dismissals. Escape is the one a keyboard reader depends on.
        expect (box->keyPressed (juce::KeyPress (juce::KeyPress::escapeKey)), "Escape was not handled");
        expect (! box->isVisible(), "Escape did not dismiss");

        logMessage ("  images in " + dir.getFullPathName());
    }
};

static AboutPartTests aboutPartTests;
