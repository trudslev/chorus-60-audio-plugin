#include "TestUtils.h"
#include "../Source/DSP/FactoryPrograms.h"

// Structural sanity only (mirrors both siblings' FactoryPresetsTests.cpp) - every entry must stay
// within each parameter's declared range. Tonal correctness is a by-ear pass once the full curated
// bank lands (see prompts/PROMPTS.md), not something this suite checks.
class FactoryProgramsTests final : public juce::UnitTest
{
public:
    FactoryProgramsTests() : juce::UnitTest("FactoryPrograms", "DSP") {}

    void runTest() override
    {
        beginTest("There are exactly 3 baseline factory programs");
        {
            expectEquals(kNumFactoryPrograms, 3);
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

        beginTest("Exactly one of the three programs is engine1-only, one engine2-only, one both");
        {
            int engine1Only = 0, engine2Only = 0, both = 0, neither = 0;
            for (const auto& p : kFactoryPrograms)
            {
                if (p.engine1 && p.engine2) ++both;
                else if (p.engine1) ++engine1Only;
                else if (p.engine2) ++engine2Only;
                else ++neither;
            }

            expectEquals(engine1Only, 1);
            expectEquals(engine2Only, 1);
            expectEquals(both, 1);
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

        beginTest("defaultFactoryProgramIndex names \"I\" and matches Parameters.h's own defaults");
        {
            expectEquals(juce::String(kFactoryPrograms[(size_t) defaultFactoryProgramIndex].name), juce::String("I"));
            const auto& p = kFactoryPrograms[(size_t) defaultFactoryProgramIndex];
            expect(p.engine1 && ! p.engine2);
        }
    }
};

static FactoryProgramsTests factoryProgramsTests;
