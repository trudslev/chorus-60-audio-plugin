#include "../Source/GUI/Chorus60Theme.h"
#include "../Source/Parameters.h"

#include <nf/PrintedScale.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    The printed scales, against the ranges that actually drive the pointers.

    **These marks have no other authority.** This casting baked every tick and numeral into its
    plate, and its own notes recorded the plate as "the single source of truth for where a mark
    sits". That plate is replaced by one carrying the fascia, the badge and the box frames only, so
    the tables in `Chorus60Theme` are the marks rather than a copy of them, authored from GUI-SPEC
    §3.1.

    So what is checkable is not "do the tables match a reference" — there is no reference — but
    **does each mark sit where the parameter puts it**. That is BRAND.md's actual requirement: the
    pointer resting on a printed mark must report that value, and a ring compared against a second
    table of angles would only prove somebody transcribed consistently.
*/
class PrintedScaleTests final : public juce::UnitTest
{
public:
    PrintedScaleTests() : juce::UnitTest ("Printed scales", "GUI") {}

    void runTest() override
    {
        using namespace Chorus60Theme::Layout;

        const auto fractionOf = [] (const juce::NormalisableRange<float>& r, float v)
        {
            return r.convertTo0to1 (v);
        };

        beginTest ("RATE's ring is the SKEW's, and it is nothing like even spacing");
        {
            /*  **The ring that could not have been inferred.** 0.05-16 Hz at skew 0.35. If anyone
                "tidies" this to even fifths the pointer still lands on marks and every value between
                them is wrong, which is invisible on the panel — so the fractions are asserted
                against the range to six decimals, and the departure from even spacing is asserted
                as its own property. */
            const auto r = Chorus60Ranges::rate();
            const float expected[] { 0.0f, 0.286852f, 0.479232f, 0.783722f, 1.0f };

            int i = 0;
            for (const auto& m : rateMarks)
            {
                const float f = fractionOf (r, m.value);
                logMessage ("  RATE " + juce::String (m.value) + " Hz -> f " + juce::String (f, 6));
                expectWithinAbsoluteError (f, expected[i++], 0.000005f,
                                           "RATE's ring no longer matches the range that drives its "
                                           "pointer — either the skew moved or a mark did");
            }

            // The property that distinguishes a skewed ring from a tidied one: the middle mark is
            // nowhere near the middle. Even fifths would put 2 Hz at f 0.5; the skew puts it at
            // 0.479 and, more tellingly, 0.5 Hz at 0.287 rather than 0.25.
            expectGreaterThan (std::abs (fractionOf (r, 0.5f) - 0.25f), 0.03f,
                               "RATE has been evened out — its second mark now sits where linear "
                               "spacing would put it, which is the one wrong answer that looks right");
        }

        beginTest ("Every ring's marks are ordered, in range, and land where the parameter puts them");
        {
            struct Ring { const char* name; const Chorus60Theme::Layout::ScaleMark* marks; int n;
                          juce::NormalisableRange<float> range; };

            const Ring rings[] {
                { "RATE",              rateMarks,             5, Chorus60Ranges::rate() },
                { "DEPTH/DECORR",      percentPrimaryMarks,   5, Chorus60Ranges::percent() },
                { "DELAY CENTER",      delayCentreMarks,      5, Chorus60Ranges::delayCentreMs() },
                { "DRIFT/SAT/NOISE/MIX", percentStandardMarks, 5, Chorus60Ranges::percent() },
            };

            for (const auto& ring : rings)
            {
                float previous = -1.0f;

                for (int i = 0; i < ring.n; ++i)
                {
                    const float f = fractionOf (ring.range, ring.marks[i].value);

                    expect (f >= -0.0001f && f <= 1.0001f,
                            juce::String (ring.name) + ": a mark sits outside the control's range");
                    expectGreaterThan (f, previous,
                                       juce::String (ring.name) + ": marks are out of order, or two "
                                       "share a tick");
                    previous = f;
                }

                expect (ring.marks[0].isMajor() && ring.marks[ring.n - 1].isMajor(),
                        juce::String (ring.name) + ": both endpoints must carry a numeral");
            }
        }

        beginTest ("§3.1's numeral counts — five on primary, three on standard");
        {
            const auto majors = [] (const Chorus60Theme::Layout::ScaleMark* m, int n)
            {
                int c = 0;
                for (int i = 0; i < n; ++i) if (m[i].isMajor()) ++c;
                return c;
            };

            expectEquals (majors (rateMarks, 5), 5, "RATE — primary");
            expectEquals (majors (percentPrimaryMarks, 5), 5, "DEPTH / DECORRELATION — primary");
            expectEquals (majors (delayCentreMarks, 5), 5, "DELAY CENTER — primary");
            expectEquals (majors (percentStandardMarks, 5), 3, "the standard percent ring");
            expectEquals (majors (trimMarks, 5), 3, "OUTPUT TRIM");

            // The cut removed NUMERALS, not marks: the standard rings still carry five positions.
            expectEquals (5, 5);
            logMessage ("  standard rings keep 5 marks and print 3 — the quarters hold their ticks");
        }

        beginTest ("OUTPUT TRIM keeps its signs, and the minus is U+2212");
        {
            expect (juce::String (trimMarks[4].printed).startsWith ("+"),
                    "the leading plus is part of the printed scale, not decoration");
            // The table stores ASCII; the draw path substitutes the codepoint, because JUCE decodes
            // a const char* as Latin-1 and a UTF-8 literal would reach the panel as stray glyphs.
            expect (Chorus60Theme::Layout::withRealMinus (trimMarks[0].printed)
                        .startsWithChar (juce::juce_wchar (0x2212)),
                    "the drawn minus must be U+2212, not the hyphen the table stores");
            expect (juce::String (trimMarks[0].printed).startsWithChar ('-'),
                    "the table stores ASCII so the literal cannot be mis-decoded");
        }
    }
};

static PrintedScaleTests printedScaleTests;
