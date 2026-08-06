#include "TestUtils.h"
#include "../Source/DSP/FactoryPrograms.h"

// Structural sanity (mirrors both siblings' FactoryPresetsTests.cpp) - every entry must stay within
// each parameter's declared range - plus the bank's own core invariant: every program has to hold up
// under ANY engine combination, so both engines' Rate/Depth must be real, usable values in every
// program regardless of which engines it ships engaged. Whether a program sounds *good* is still a
// by-ear judgement this suite can't make; whether engaging an engine can produce silence is exactly
// the kind of thing it can.
class FactoryProgramsTests final : public juce::UnitTest
{
public:
    FactoryProgramsTests() : juce::UnitTest("FactoryPrograms", "DSP") {}

    void runTest() override
    {
        beginTest("There are exactly 9 factory programs");
        {
            expectEquals(kNumFactoryPrograms, 9);
        }

        beginTest("Every program's fields are within Parameters.h's declared ranges");
        {
            // Rate I and II share the slow 0.05-8Hz range; I+II has its own wider 0.05-16Hz one
            // because the bank specifies 9.75, 11 and 14Hz there. Checking them against the same
            // ceiling would either reject valid I+II data or silently permit an out-of-range I/II
            // value, so they are asserted separately and deliberately.
            const auto checkConfiguration = [this] (const FactoryConfiguration& c,
                                                    float maxRateHz,
                                                    const juce::String& where)
            {
                expect(c.rateHz >= 0.05f && c.rateHz <= maxRateHz, where + " rate");
                expect(c.depthPercent >= 0.0f && c.depthPercent <= 100.0f, where + " depth");
                expect(c.delayCentreMs >= 2.0f && c.delayCentreMs <= 14.0f, where + " centre");
                expect(c.decorrelationPercent >= 0.0f && c.decorrelationPercent <= 100.0f,
                       where + " decorrelation");
            };

            for (const auto& p : kFactoryPrograms)
            {
                checkConfiguration(p.configI, 8.0f, juce::String(p.name) + " I");
                checkConfiguration(p.configII, 8.0f, juce::String(p.name) + " II");
                checkConfiguration(p.configBoth, 16.0f, juce::String(p.name) + " I+II");

                expect(p.driftPercent >= 0.0f && p.driftPercent <= 100.0f, p.name);
                expect(p.saturationPercent >= 0.0f && p.saturationPercent <= 100.0f, p.name);
                expect(p.noisePercent >= 0.0f && p.noisePercent <= 100.0f, p.name);
                expect(p.mixPercent >= 0.0f && p.mixPercent <= 100.0f, p.name);
                expect(p.trimDb >= -12.0f && p.trimDb <= 12.0f, p.name);
            }
        }

        // The correction in design/BBD-TECHNICAL-NOTES-ADDENDUM.md is a claim about what I+II *is*:
        // a distinct fast configuration, not I and II summed. If a program's I+II rate ever drifted
        // down among its I and II rates, the bank would silently stop expressing that - so assert
        // the structural relationship rather than trusting the table to stay correct by inspection.
        beginTest("I+II is genuinely a faster configuration, not a blend of I and II");
        {
            for (const auto& p : kFactoryPrograms)
            {
                const float slowest = juce::jmax(p.configI.rateHz, p.configII.rateHz);
                expect(p.configBoth.rateHz > slowest * 2.0f,
                       juce::String(p.name) + " I+II rate is not clearly faster than I and II");
                expect(p.configBoth.delayCentreMs < juce::jmin(p.configI.delayCentreMs,
                                                              p.configII.delayCentreMs),
                       juce::String(p.name) + " I+II is not centred on a narrower delay");
            }
        }

        // The bank's core invariant. A program ships with some engine combination engaged, but the
        // latches are front-panel controls the player is expected to hit at will - so the values for
        // a *currently disengaged* engine matter just as much as the engaged one's. A zero (or
        // out-of-range) Depth on the idle engine would mean engaging it yields no modulation at all,
        // which is the silent-engine failure this bank exists to rule out.
        beginTest("Every program carries usable Rate/Depth for ALL THREE configurations");
        {
            for (const auto& p : kFactoryPrograms)
            {
                const auto usable = [this, &p] (const FactoryConfiguration& c, const char* which)
                {
                    expect(c.depthPercent > 0.0f,
                           juce::String(p.name) + " has no Depth " + which);
                    expect(c.rateHz > 0.0f,
                           juce::String(p.name) + " has no Rate " + which);
                };
                usable(p.configI, "I");
                usable(p.configII, "II");
                usable(p.configBoth, "I+II");
            }
        }

        beginTest("At least one program ships in each engine combination, and none ships silent");
        {
            int engine1Only = 0, engine2Only = 0, both = 0, neither = 0;
            for (const auto& p : kFactoryPrograms)
            {
                if (p.engine1 && p.engine2) ++both;
                else if (p.engine1) ++engine1Only;
                else if (p.engine2) ++engine2Only;
                else ++neither;
            }

            expect(engine1Only > 0);
            expect(engine2Only > 0);
            expect(both > 0);
            // Every program is a *chorus* program - shipping one with both engines idle would put
            // the plugin in bypass on load.
            expectEquals(neither, 0);
        }

        beginTest("Program names are unique");
        {
            juce::StringArray names;
            for (const auto& p : kFactoryPrograms)
                names.add(p.name);
            names.sort(false);
            for (int i = 1; i < names.size(); ++i)
                expect(names[i] != names[i - 1], names[i]);
        }

        // Fresh instantiation has to land on EIGHTY-TWO, and the LCD renders the bank position
        // 1-based - so this is the program that must surface as "01 EIGHTY-TWO".
        beginTest("defaultFactoryProgramIndex is EIGHTY-TWO at bank position 01");
        {
            expectEquals(defaultFactoryProgramIndex, 0);

            const auto& p = kFactoryPrograms[(size_t) defaultFactoryProgramIndex];
            expectEquals(juce::String(p.name), juce::String("EIGHTY-TWO"));
            expect(p.engine1 && ! p.engine2);
        }
    }
};

static FactoryProgramsTests factoryProgramsTests;
