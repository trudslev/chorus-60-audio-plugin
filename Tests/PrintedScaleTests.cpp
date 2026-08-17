#include "../Source/GUI/Chorus60Theme.h"
#include "../Source/Parameters.h"

#include <nf/PrintedScale.h>

#include <juce_audio_processors/juce_audio_processors.h>

#include <map>

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

        beginTest ("§3's knob geometry — checked one at a time, because the row is not uniform");
        {
            using namespace Chorus60Theme::Layout;

            /*  **Three of five standard knobs stay put and two move, which is the shape that gets
                missed.** Checked as a group, DRIFT/SATURATION/NOISE landing where they always did
                makes MIX and OUTPUT TRIM look like transcription slips. Each is asserted against §3
                individually for that reason. */
            expectWithinAbsoluteError (modKnobD, 76.0f, 0.01f, "primary class");
            expectWithinAbsoluteError (globalKnobD, 56.0f, 0.01f, "standard class");
            expectWithinAbsoluteError (modKnobCentreY, 416.0f, 0.01f, "primary pivot row");
            expectWithinAbsoluteError (globalKnobCentreY, 660.0f, 0.01f, "standard pivot row");

            const float primaryX[] { 425.0f, 621.0f, 817.0f, 1013.0f };
            for (size_t i = 0; i < modKnobCentreX.size(); ++i)
                expectWithinAbsoluteError (modKnobCentreX[i], primaryX[i], 0.01f,
                                           "primary knob " + juce::String ((int) i));

            struct Row { const char* id; float x; };
            const Row standard[] { {"drift", 389.0f}, {"saturation", 569.0f}, {"noise", 749.0f},
                                   {"mix", 1006.0f}, {"trim", 1186.0f} };

            for (const auto& r : standard)
            {
                bool found = false;

                for (const auto& k : knobs)
                    if (juce::String (k.paramID) == r.id)
                    {
                        found = true;
                        expectWithinAbsoluteError (k.cx, r.x, 0.01f,
                                                   juce::String (r.id) + " is not at §3's x");
                    }

                expect (found, juce::String (r.id) + " is missing from the knob table");
            }

            // Every knob carries a ring, and both endpoints of each are numeralled.
            for (const auto& k : knobs)
            {
                expect (k.scale.marks != nullptr && k.scale.count > 1,
                        juce::String (k.paramID) + " has no printed scale");
                expect (k.scale.marks[0].isMajor() && k.scale.marks[k.scale.count - 1].isMajor(),
                        juce::String (k.paramID) + ": an endpoint lost its numeral");
            }

            for (const auto& s : modKnobScales)
                expect (s.marks != nullptr && s.count == 5, "a mod ring is missing its marks");
        }

        beginTest ("All NINE rings have a drawing site, named one at a time");
        {
            /*  **The enumeration's second column, struck per row rather than per pass.** A paint
                pass that draws "the rings" looks complete on a panel while one knob silently has
                none — the knob still draws, the pointer still moves, and only a comparison against
                the old plate would show it. So this walks the same list the panel paints from and
                names any knob missing from it, rather than asserting the pass ran.

                It counts what the painter PRODUCED, too: `drawKnobScale` returns how many majors it
                numeralled, so a ring that is present in the list and draws nothing is caught as
                well as one that is absent from it. */
            const auto rings = Chorus60Theme::ringsToDraw();

            expectEquals ((int) rings.size(), 9, "the panel does not paint nine rings");

            const char* const expected[] { "rate", "depth", "delayCenter", "decorrelation",
                                           "drift", "saturation", "noise", "mix", "trim" };

            for (const auto* id : expected)
            {
                bool found = false;

                for (const auto& r : rings)
                    if (juce::String (r.paramID) == id)
                        found = true;

                expect (found, juce::String (id) + " has NO ring in the paint pass. Its knob will "
                               "still draw and its pointer will still move, so nothing on the panel "
                               "shows this");
            }

            // Drive the painter into a real context and count what each ring produced.
            juce::Image target { juce::Image::ARGB, 1340, 812, true };
            juce::Graphics g { target };

            for (const auto& r : rings)
            {
                const int numeralled = Chorus60Theme::drawKnobScale (g, r);

                logMessage ("  " + juce::String (r.paramID) + ": "
                            + juce::String (r.scale.count) + " marks, "
                            + juce::String (numeralled) + " numeralled");

                expectGreaterThan (numeralled, 0,
                                   juce::String (r.paramID) + "'s ring drew no numerals at all");
                expect (numeralled == 3 || numeralled == 5,
                        juce::String (r.paramID) + ": §3.1 gives five numerals on primary and three "
                        "on standard, and this is neither");
            }
        }

        beginTest ("The three box layers PARTITION the nine rings — none lost, none drawn twice");
        {
            /*  Each box draws its own printed layer now, because §7.2 dims the box and anything
                painted over it from outside would stay bright. That splits one paint pass into
                three, and a split is where a ring goes missing while every box still looks
                populated — so the split is asserted rather than eyeballed.

                **`ringsInBox` filters by containment**, so this is a real check on the geometry and
                not a restatement of a hand-written assignment: a knob whose box moves out from
                under it lands in no layer, and that is exactly what this fails on. */
            const auto all = Chorus60Theme::ringsToDraw();

            std::map<juce::String, int> timesDrawn;
            for (const auto& r : all)
                timesDrawn[r.paramID] = 0;

            int total = 0;
            for (const auto& box : Chorus60Theme::Layout::groupBoxes)
            {
                const auto inside = Chorus60Theme::ringsInBox (box);
                logMessage ("  " + juce::String (box.title) + ": "
                            + juce::String ((int) inside.size()) + " rings");

                for (const auto& r : inside)
                    ++timesDrawn[r.paramID];

                total += (int) inside.size();
                expectGreaterThan ((int) inside.size(), 0,
                                   juce::String (box.title) + " draws no rings at all");
            }

            expectEquals (total, (int) all.size(),
                          "the three layers do not sum to the nine rings the panel paints");

            for (const auto& [id, count] : timesDrawn)
            {
                expect (count != 0, id + " falls in NO box, so nothing draws its ring, its numerals, "
                                         "its unit or its label — and its knob still draws");
                expect (count <= 1, id + " falls in two boxes and is drawn twice");
            }
        }

        beginTest ("The label row is §3's, not the derived figure it replaced");
        {
            /*  **THE FIGURE THIS PINS WAS 16 PX OUT, and the arm is here because the derivation
                that produced it was sound.** `modLabelRowY` was 504: the previous 464 plus the 40
                the pivot had moved, holding the label's gap to the knob because no spec restated
                this casting's mod cell. The delivered prototype restates it as an offset inside the
                knob's own box — `label top: d + 34` — which for the primary row is 488.

                Read this as **catching divergence, not asserting provenance**: a re-typed 488 and
                `cy + r + 34` are indistinguishable while they agree. What it buys is the moment the
                offset moves and one of the two rows does not follow. */
            using namespace Chorus60Theme::Layout;

            expectEquals (knobLabelTop (modKnobCentreY, modKnobD), 488.0f,
                          "the primary label row moved off 416 + 38 + 34");
            expectEquals (knobLabelTop (globalKnobCentreY, globalKnobD), 722.0f,
                          "the standard label row moved off 660 + 28 + 34");
            expectEquals (knobUnitTop (modKnobCentreY, modKnobD), 474.0f,
                          "the primary unit row moved off 416 + 38 + 20");

            /*  **SHOWN ABLE TO FAIL, and the failure named the defect exactly.** Perturbing
                `knobLabelTopOffset` 34 → 50 turned the first two arms red at **504** and **738** —
                and 504 is precisely the figure this replaced, which places the old derivation's
                error at 16 px rather than merely somewhere.

                **A FOURTH ARM WAS WRITTEN HERE AND DELETED RATHER THAN KEPT.** It asserted that the
                two classes share one gap under the cap — `labelTop − (cy + r)` equal on Ø76 and
                Ø56 — which reads like a real invariant and cannot fail: both sides compute from the
                single `knobLabelTopOffset`, so it compares that constant with itself. That is this
                suite's recorded tell for a check whose input comes from the thing it checks, and it
                survived writing because the property it names IS true and IS load-bearing. What
                makes it unassertable is that it is true *by construction* — there is one constant,
                so there is nothing that could disagree. */
        }

        beginTest ("The nameplate stack closes on the shared descriptor anchor");
        {
            /*  Three lines in core's 303 x 84 zone, and the middle one is the anchor all six
                castings share. This casting's wordmark height and leading are its own — §I keeps
                the nameplate per casting, because six metaphors are six paint routines — so what is
                checkable is that its own stack *lands* where §4 says.

                **The model line is core's `modelLineY`, and the derivation that used to define it is
                the arm.** `descriptorY + descriptorH` is 95 and so is core's constant; having both
                as definitions would be two sources for one figure, which is the defect this
                casting's LCD budget spent a round removing. As an assertion it costs nothing and
                fires the moment §4 moves one without the other. */
            using namespace Chorus60Theme::Layout;

            expect (nf::HeaderGeometry::landsOnDescriptorAnchor ((int) nameplateY,
                                                                 (int) wordmarkLineBox,
                                                                 (int) nameplateLeading),
                    "the wordmark's line box plus its leading no longer lands the descriptor on "
                    "§4's shared anchor");

            expectEquals (modelLineY, (float) (nf::HeaderGeometry::descriptorY
                                                 + nf::HeaderGeometry::descriptorH),
                          "core's modelLineY and descriptorY + descriptorH have diverged");

            // The wordmark is a literal here and CHORUS60_PRODUCT_NAME in CMakeLists, because
            // JucePlugin_Name is not defined in this target. Nothing can check the two agree from
            // inside the test binary, so this only pins the spelling this file draws.
            expectEquals (juce::String (wordmarkText), juce::String ("CHORUS-60"));
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
