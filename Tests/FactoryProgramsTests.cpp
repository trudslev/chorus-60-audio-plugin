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
            for (const auto& p : kFactoryPrograms)
            {
                expect(p.rate1Hz >= 0.2f && p.rate1Hz <= 2.0f, p.name);
                expect(p.depth1Percent >= 0.0f && p.depth1Percent <= 100.0f, p.name);
                expect(p.rate2Hz >= 0.2f && p.rate2Hz <= 2.0f, p.name);
                expect(p.depth2Percent >= 0.0f && p.depth2Percent <= 100.0f, p.name);
                expect(p.delayCenterMs >= 5.0f && p.delayCenterMs <= 15.0f, p.name);
                expect(p.decorrelationPercent >= 0.0f && p.decorrelationPercent <= 100.0f, p.name);
                expect(p.driftPercent >= 0.0f && p.driftPercent <= 100.0f, p.name);
                expect(p.saturationPercent >= 0.0f && p.saturationPercent <= 100.0f, p.name);
                expect(p.noisePercent >= 0.0f && p.noisePercent <= 100.0f, p.name);
                expect(p.mixPercent >= 0.0f && p.mixPercent <= 100.0f, p.name);
                expect(p.trimDb >= -24.0f && p.trimDb <= 24.0f, p.name);
            }
        }

        // The bank's core invariant. A program ships with some engine combination engaged, but the
        // latches are front-panel controls the player is expected to hit at will - so the values for
        // a *currently disengaged* engine matter just as much as the engaged one's. A zero (or
        // out-of-range) Depth on the idle engine would mean engaging it yields no modulation at all,
        // which is the silent-engine failure this bank exists to rule out.
        beginTest("Every program carries usable Rate/Depth for BOTH engines, engaged or not");
        {
            for (const auto& p : kFactoryPrograms)
            {
                expect(p.depth1Percent > 0.0f, juce::String(p.name) + " has no Depth I");
                expect(p.depth2Percent > 0.0f, juce::String(p.name) + " has no Depth II");
                expect(p.rate1Hz > 0.0f, juce::String(p.name) + " has no Rate I");
                expect(p.rate2Hz > 0.0f, juce::String(p.name) + " has no Rate II");
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
