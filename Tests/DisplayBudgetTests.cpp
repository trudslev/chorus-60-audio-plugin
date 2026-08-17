#include "../Source/GUI/Chorus60Theme.h"
#include "../Source/DSP/ProgramManager.h"

#include <juce_gui_basics/juce_gui_basics.h>

/**
    The LCD's character budget, measured off the face the paint path draws.

    **This exists because the panel carried THREE figures for one quantity and nothing bound them.**
    `lcdCharacterBudget` was the 352 px cell; the paint path trimmed 26 px and drew into 326; the
    naming field used a different inset again at 328; and `readoutFormat()` handed core the cell
    figure, so the live readout believed it had characters it did not. The cap was derived from the
    middle one, which made the only figure constraining what a user can type the only one nobody
    could find from a constant's name.

    So the assertions below bind the **drawn run** to the **cap plus the dirty marker**, not to the
    budget alone. That is the comparison that would have caught it: a run wide enough for the cap is
    not wide enough if an edited Program appends " *".
*/
class DisplayBudgetTests final : public juce::UnitTest
{
public:
    DisplayBudgetTests() : juce::UnitTest ("PROGRAM display budget", "gui") {}

    void runTest() override
    {
        using namespace Chorus60Theme;

        beginTest ("The stated advance is what the drawn face actually advances");
        {
            /*  The one term that belongs to the FONT rather than to the cell, so it is measured
                rather than taken from the theme — which keeps the two sides of every assertion
                below coming from different places. */
            const auto font = Chorus60Theme::monoFont (
                                  Chorus60Theme::monoFontHeightForCssPx (Layout::lcdCssPx));
            const juce::String sample ("MMMMMMMMMMMMMMMMMMMM");

            const float measured = (Chorus60Theme::trackedTextWidth (sample, font, Layout::lcdTrackingPx)
                                        / (float) sample.length());
            const float stated = Layout::lcdGlyphAdvance + Layout::lcdTrackingPx;

            logMessage ("  stated " + juce::String (stated, 2) + " px/char, measured "
                        + juce::String (measured, 2));

            expectWithinAbsoluteError (measured, stated, 0.25f,
                                       "the theme's advance no longer describes the face the paint "
                                       "path draws, so every figure derived from it is describing a "
                                       "different font");
        }

        beginTest ("The DRAWN RUN holds the cap PLUS the dirty marker");
        {
            const int fits = Layout::lcdCharacterBudget;

            logMessage ("  cell " + juce::String (Layout::programNameCellW, 0)
                        + " - padding " + juce::String (Layout::lcdNameRightPadding, 0)
                        + " = run " + juce::String (Layout::lcdDrawnRunW, 0)
                        + " -> " + juce::String (fits) + " characters; cap "
                        + juce::String (ProgramManager::maxProgramNameLength)
                        + " + marker " + juce::String (Layout::lcdDirtyMarkerChars));

            expectGreaterOrEqual (fits,
                                  ProgramManager::maxProgramNameLength + Layout::lcdDirtyMarkerChars,
                                  "a name at the cap plus its \" *\" does not fit the run the panel "
                                  "draws into. The cap may never shrink, so this cannot be corrected "
                                  "after anyone has saved against it");
        }

        beginTest ("The two copies of the cap have not drifted");
        {
            expectEquals (Layout::maxProgramNameLength, ProgramManager::maxProgramNameLength,
                          "ProgramManager cannot include a GUI header, so this binding is the only "
                          "thing keeping its cap and the panel's equal");
        }

        beginTest ("The readout is told the DRAWN run, not the cell");
        {
            // readoutFormat() used to hand core the cell figure, which is wider than what is drawn.
            const auto f = Chorus60Theme::readoutFormat();

            expectEquals (f.nameCharacterBudget, Layout::lcdCharacterBudget,
                          "core is being told a budget the panel does not draw into");
        }
    }
};

static DisplayBudgetTests displayBudgetTests;
