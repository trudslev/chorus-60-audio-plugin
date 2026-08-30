#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include "../Source/GUI/ProgramHeader.h"

#include <nf/testing/ComponentRender.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    **The name being typed must be visible, and its caret must blink.**

    Both halves shipped broken in v1.0.0-rc.1 and both were reported from a real panel on
    2026-08-30, by a person naming a Program. Six green suites and a three-platform CI matrix
    missed them, because nothing anywhere asserted that naming mode renders anything at all.

      * TapeRot drew the CURRENT Program's name in the field while the caption above it read
        NAME PROGRAM. Keystrokes were recorded and rendered nowhere, so the field looked dead and
        a second SAVE press committed an empty name as `TAKE n`.
      * CHORUS-60 drew the caret into a cached image whose key holds what varies - bank, name,
        modified, naming state, typed text, menu, readout - and a clock phase is not among them.
        The layer was rebuilt on entering naming and on each keystroke and never between, so the
        caret appeared about half the time on entry and never blinked.

    **Why this samples over a period rather than comparing two renders.** The blink is 1 s at
    50 percent duty off the wall clock, which a test cannot set. Two shots 500 ms apart can land on
    the same phase if the sleep drifts across a boundary; five shots spanning 1.2 s cannot all miss.
    The assertion is that at least two of them DIFFER, which is exactly the property that was absent
    - a frozen caret gives five identical images no matter when they are taken.

    What this cannot see is what `renderComponent` cannot see: focus, the cursor the window server
    owns, z-order. It asserts that the panel's own pixels change, which is the half that broke.
*/
class NamingCaretTests final : public juce::UnitTest
{
public:
    NamingCaretTests() : juce::UnitTest ("Naming caret", "GUI") {}

    /** Pixels differing between two same-sized renders. */
    static int pixelsDiffering (const juce::Image& a, const juce::Image& b)
    {
        if (a.getWidth() != b.getWidth() || a.getHeight() != b.getHeight())
            return -1;

        const juce::Image::BitmapData pa (a, juce::Image::BitmapData::readOnly);
        const juce::Image::BitmapData pb (b, juce::Image::BitmapData::readOnly);

        int n = 0;
        for (int y = 0; y < a.getHeight(); ++y)
            for (int x = 0; x < a.getWidth(); ++x)
                if (pa.getPixelColour (x, y) != pb.getPixelColour (x, y))
                    ++n;
        return n;
    }

    void runTest() override
    {
        beginTest ("the typed name is drawn, and the caret blinks");

        Chorus60AudioProcessor processor;
        auto* editor = dynamic_cast<Chorus60AudioProcessorEditor*> (processor.createEditor());
        expect (editor != nullptr, "no editor");
        std::unique_ptr<juce::AudioProcessorEditor> owner { editor };

        ProgramHeader* header = findHeader (editor);
        expect (header != nullptr, "no ProgramHeader under the editor");

        if (header == nullptr)
            return;

        header->beginNamingForTest();

        // Five shots across 1.2 s - more than one full blink period, so no arrangement of
        // sleep drift can sample the same phase every time.
        std::vector<juce::Image> shots;
        for (int i = 0; i < 5; ++i)
        {
            shots.push_back (nf::testing::renderComponent (*header));
            juce::Thread::sleep (300);
        }

        int mostDifferent = 0;
        for (size_t i = 1; i < shots.size(); ++i)
            mostDifferent = juce::jmax (mostDifferent, pixelsDiffering (shots.front(), shots[i]));

        expect (mostDifferent > 0,
                "the panel is identical across a full blink period while naming - the caret is not "
                "animating. It was drawn into the cached static layer, whose key has no clock "
                "phase, so it only changes when the typed text does.");

        logMessage ("  most-changed pixels across the period: " + juce::String (mostDifferent));

        // And the typed text itself must reach the panel: type a character and the pixels must
        // change beyond whatever the caret alone does.
        const auto before = nf::testing::renderComponent (*header);
        header->keyPressed (juce::KeyPress ('A', juce::ModifierKeys(), 'A'));
        const auto after = nf::testing::renderComponent (*header);

        const int typedDelta = pixelsDiffering (before, after);
        expect (typedDelta > 0,
                "typing a character changed no pixels - the field is not rendering typedName at "
                "all, which is what TapeRot shipped: the current Program's name stayed in the "
                "cell while keystrokes went nowhere.");

        logMessage ("  pixels changed by typing one character: " + juce::String (typedDelta));
    }

private:
    static ProgramHeader* findHeader (juce::Component* root)
    {
        if (auto* h = dynamic_cast<ProgramHeader*> (root))
            return h;

        for (int i = 0; i < root->getNumChildComponents(); ++i)
            if (auto* h = findHeader (root->getChildComponent (i)))
                return h;

        return nullptr;
    }
};

static NamingCaretTests namingCaretTests;
