#include "../Source/GUI/Chorus60MenuLookAndFeel.h"

#include <nf/MenuMetrics.h>

#include <juce_gui_basics/juce_gui_basics.h>

/**
    The Program list's caption is sized from its own type, never from the row.

    **The number is not the thing being guarded — the construction is.** This panel's caption comes
    out 19, the same as Elmer's, and that is a coincidence: Share Tech Mono's line box is 1.127 em
    against IBM Plex Mono's 1.300, and 11px of one plus 9px of the other happen to meet once the
    3/4 padding is added. So the test asserts the construction reproduces, and asserts the
    relationship that the ruling is actually about, rather than pinning a literal that would go
    stale the moment the caption type changed.
*/
class MenuMetricsTests final : public juce::UnitTest
{
public:
    MenuMetricsTests() : juce::UnitTest ("Menu metrics", "GUI") {}

    void runTest() override
    {
        // Read from a constructed instance rather than the private factory: getMenuMetrics()
        // is core's public accessor, and reading it back off a live look-and-feel is what the
        // menu itself does.
        Chorus60MenuLookAndFeel lookAndFeel;
        const auto m = lookAndFeel.getMenuMetrics();

        logMessage ("  caption " + juce::String (m.sectionHeaderHeight)
                    + "px, row " + juce::String (m.rowHeight)
                    + "px, separator " + juce::String (m.separatorHeight) + "px");

        beginTest ("The caption is its padding plus its own type's line box");
        {
            const auto captionFont = Chorus60Theme::monoFont (11.0f);

            expectEquals (m.sectionHeaderHeight, nf::captionHeight (captionFont, 3, 4));

            // The line box is LOGGED rather than pinned to a literal, because the castings do not
            // all build fonts the same way: most pass a CSS px through withPointHeight, so the
            // height is the face's own line box (Share Tech Mono 1.127 em, Plex Mono 1.300), while
            // Chorus-60's monoFont takes a JUCE height directly and has a separate
            // monoFontHeightForCssPx converter. Asserting one ratio here would fail on a casting
            // whose caption is correctly a different size.
            //
            // What matters is that the caption is built from the font the panel DRAWS, whatever
            // that font's construction - which is what the assertion above checks.
            logMessage ("  caption line box " + juce::String (captionFont.getHeight(), 3) + "px");
        }

        beginTest ("The caption is SHORTER than a row, which is the ruling");
        {
            // JUCE's default is rowHeight + rowHeight / 2 - a caption half again taller than a row,
            // which is a menu convention rather than this panel's. Both designer-authored captions
            // in the suite are shorter than their rows (Elmer 19 against 22, Reflect-84 22 against
            // 26); this is what stops the inherited value coming back.
            expect (m.sectionHeaderHeight < m.rowHeight,
                    "caption " + juce::String (m.sectionHeaderHeight)
                        + " should be under the row's " + juce::String (m.rowHeight));

            expect (m.sectionHeaderHeight != m.rowHeight + m.rowHeight / 2,
                    "the caption is back to JUCE's row-and-a-half");
        }

        beginTest ("A row never grows to the platform's standard item height");
        {
            int w = 0, h = 0;
            auto& lf = lookAndFeel;

            lf.getIdealPopupMenuItemSize ("ROOM", false, 0, w, h);
            expectEquals (h, m.rowHeight);

            lf.getIdealPopupMenuItemSize ("ROOM", false, 40, w, h);
            expectEquals (h, m.rowHeight, "the row grew to the platform's standard item height");
        }
    }
};

static MenuMetricsTests menuMetricsTests;
