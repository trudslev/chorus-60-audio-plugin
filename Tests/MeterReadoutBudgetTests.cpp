#include "../Source/GUI/Chorus60Theme.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <vector>

/**
    The IN/OUT readout's character budget.

    **Suite ruling 2026-08-14: floor sentinel, +99.9 ceiling, one decimal always, no plus at exactly
    0.0 dB.** The widest string is then FIVE as a GUARANTEE rather than as a range, and that
    distinction is the point: the question a well's width has to answer is not what the readout has
    been observed to print but what it can ever be ASKED to print. An observed idle string is the
    same class of evidence as an uncrashing over-delivery.

    **No readout in ANY of the six castings had a ceiling** — the widest string was bounded only by
    how loud the signal got. Gatecrasher is the evidence nobody considered the numerals needed one:
    it carries a meter ceiling constant that feeds its BAR while its readout ignored it. Reusing that
    constant would have been wrong in the other direction, clamping a genuine +6 dB reading to "0.0",
    so the readout has its own.

    **And the sign at exactly 0.0 dB was one value with two answers.** Chorus-60 printed the plus at
    `db >= 0.0f` and Gatecrasher at `db > 0.0f`, so at exactly unity one read "+0.0" and the other
    "0.0" — no reason behind either, just whichever comparison somebody typed. The plus means ABOVE
    unity, and 0.0 dB is not.
*/
class MeterReadoutBudgetTests final : public juce::UnitTest
{
public:
    MeterReadoutBudgetTests() : juce::UnitTest ("Meter readout budget", "GUI") {}

    void runTest() override
    {
        beginTest ("Never wider than five characters, anywhere a level can reach");
        {
            std::vector<float> probes;

            // Dense through the whole displayable range, where any sentinel boundary lives, and far
            // past full scale, where the missing ceiling was. A coarse sweep steps over a narrow
            // band without touching it — which is how TapeRot's 0.58 %-wide "-100.0" band survived
            // being looked at until a 1e-8 sweep went through it.
            for (int i = -1200; i <= 1200; ++i)
                probes.push_back ((float) i * 0.1f);

            for (float v : { -1.0e6f, -99.95f, -99.9f, -60.05f, -60.0f, -0.049f, 0.0f,
                             0.049f, 99.9f, 100.0f, 1.0e3f, 1.0e6f })
                probes.push_back (v);

            int widest = 0;
            juce::String widestString, atExactlyZero;

            for (float db : probes)
            {
                const auto s = Chorus60Theme::Layout::formatMeterDb (db);

                if (s.length() > widest) { widest = s.length(); widestString = s; }
                if (db == 0.0f) atExactlyZero = s;
            }

            logMessage ("  widest over " + juce::String ((int) probes.size()) + " probes: \""
                            + widestString + "\" at " + juce::String (widest) + " characters");
            logMessage ("  at exactly 0.0 dB: \"" + atExactlyZero + "\"");

            expectEquals (widest, 5,
                          "the readout exceeded its guaranteed width. The well is sized to a "
                          "GUARANTEE, not to a range, so one character over is a defect rather than "
                          "a rare case");

            expect (! atExactlyZero.startsWith ("+"),
                    "the readout printed a plus at exactly 0.0 dB. The plus means ABOVE unity and "
                    "0.0 dB is not, so it claims something false: \"" + atExactlyZero + "\"");
        }

        beginTest ("Shown able to fail — unclamped, a loud signal is wider");
        {
            /*  Without this, the row above cannot be told apart from a sweep that never reaches a
                wide value. The pre-ruling construction — no ceiling — is evaluated over the same
                loud probes and must exceed the guarantee, which also records what was wrong. */
            int widest = 0;
            juce::String widestString;

            for (float db : { 100.0f, 1.0e3f, 1.0e6f })
            {
                const auto s = juce::String (db, 1);
                if (s.length() > widest) { widest = s.length(); widestString = s; }
            }

            logMessage ("  unclamped: \"" + widestString + "\" at " + juce::String (widest)
                            + " characters");

            expectGreaterThan (widest, 5,
                               "the unclamped construction did not exceed the guarantee over these "
                               "probes, so the sweep above proves nothing about the clamp");
        }
    }
};

static MeterReadoutBudgetTests meterReadoutBudgetTests;
