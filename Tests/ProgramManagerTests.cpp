#include "TestUtils.h"

#include <set>
#include "../Source/DSP/ProgramManager.h"
#include "../Source/Parameters.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace
{
    class DummyProcessor final : public juce::AudioProcessor
    {
    public:
        const juce::String getName() const override { return "Dummy"; }
        void prepareToPlay(double, int) override {}
        void releaseResources() override {}
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
        double getTailLengthSeconds() const override { return 0.0; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return {}; }
        void changeProgramName(int, const juce::String&) override {}
        void getStateInformation(juce::MemoryBlock&) override {}
        void setStateInformation(const void*, int) override {}
    };
}

// Deliberately does not exercise initialise()/saveNewUserProgram()/deleteUserProgram(): those touch
// the real per-OS user Presets/Programs directory, which an automated test suite shouldn't write to
// or depend on the contents of - same restraint as both siblings' own ProgramManagerTests.cpp.
class ProgramManagerTests final : public juce::UnitTest
{
public:
    ProgramManagerTests() : juce::UnitTest("ProgramManager", "DSP") {}

    void runTest() override
    {
        beginTest("Factory slugs are unique, non-empty and pinned as literals");
        {
            std::set<juce::String> slugs;

            for (const auto& fp : kFactoryPrograms)
            {
                const juce::String slug { fp.slug };
                expect(slug.isNotEmpty(), juce::String(fp.name) + " has no slug");
                expect(slugs.insert(slug).second, "duplicate slug: " + slug);
                expect(! slug.containsChar(' '), "slug must be filename- and XML-safe: " + slug);
            }

            expectEquals((int) slugs.size(), (int) kFactoryPrograms.size());

            // **Literals on purpose.** A display name may be revised freely; a slug may not,
            // because it is what a saved session stores. Asserted through the struct this would
            // follow a rename silently and prove nothing.
            expect(juce::String(kFactoryPrograms[0].slug) == "eighty-two");
            expect(juce::String(kInitProgram.slug) == "init");
        }

        beginTest("Every factory position round-trips through identity");
        {
            DummyProcessor proc;
            juce::AudioProcessorValueTreeState apvts(proc, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            ProgramManager manager(apvts);

            for (int i = 0; i < kNumFactoryPrograms; ++i)
            {
                const auto id = ProgramManager::factoryIdAt(i);
                expectEquals(ProgramManager::factoryPositionOf(id.id), i);
                expectEquals(manager.getProgramName(i), juce::String(kFactoryPrograms[(size_t) i].name));

                // Host index n IS Factory Program n+1 - the alignment excluding INIT buys.
                expect(manager.displayLabelFor(id).startsWith(juce::String(i + 1).paddedLeft('0', 2)));
            }
        }

        beginTest("The host list is the Factory bank, and its size never changes");
        {
            DummyProcessor proc;
            juce::AudioProcessorValueTreeState apvts(proc, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            ProgramManager manager(apvts);

            // juce_AudioProcessor.h: the value "must not change over its lifetime".
            expectEquals(manager.getNumPrograms(), kNumFactoryPrograms);
            expect(manager.getProgramName(kNumFactoryPrograms).isEmpty(),
                   "nothing beyond the Factory bank may be addressable by position");
        }

        beginTest("The current Program defaults to the default Factory Program");
        {
            DummyProcessor proc;
            juce::AudioProcessorValueTreeState apvts(proc, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            ProgramManager manager(apvts);

            expect(manager.getCurrentProgramId() == ProgramManager::factoryIdAt(defaultFactoryProgramIndex));
            expectEquals(manager.getCurrentFactoryPosition(), defaultFactoryProgramIndex);
        }

        beginTest("setCurrentProgramWithoutApplying moves the identity without touching the APVTS");
        {
            DummyProcessor proc;
            juce::AudioProcessorValueTreeState apvts(proc, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            ProgramManager manager(apvts);

            const float before = *apvts.getRawParameterValue(ParamIDs::mix);
            manager.setCurrentProgramWithoutApplying(ProgramManager::factoryIdAt(4));

            expect(manager.getCurrentProgramId() == ProgramManager::factoryIdAt(4));
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::mix), before, 1.0e-6f);
        }

        beginTest("An unresolved identifier keeps its name for display and resolves to nothing");
        {
            DummyProcessor proc;
            juce::AudioProcessorValueTreeState apvts(proc, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            ProgramManager manager(apvts);

            const auto id = manager.resolve(ProgramBank::factory, "a-program-from-the-future",
                                             "SOME FUTURE SOUND");

            expect(id.bank == ProgramBank::unresolved);
            expect(id.displayName == "SOME FUTURE SOUND",
                   "the panel needs a presentable name - a slug would read as a rendering fault");
            expect(manager.displayLabelFor(id) == "SOME FUTURE SOUND",
                   "an unresolved Program is in no bank, so it has no number");
        }

        beginTest("The schema gate has two literal-pinned branches, not one moving comparison");
        {
            using LegacyMigration::classifySchema;
            using V = LegacyMigration::SchemaVerdict;

            // **Literals, deliberately.** The gate read `!= currentStateSchemaVersion`, which was
            // correct exactly once: every bump then discarded the previous version's sessions
            // wholesale, including ones whose parameters had not changed meaning at all.
            expect(classifySchema(1) == V::tooOld,
                   "v1 predates the paged-engine rework - its normalised values mean something else");
            expect(classifySchema(2) == V::readable, "v2 must survive the identity bump");
            expect(classifySchema(3) == V::readable, "v3 must survive the identity bump");
            expect(classifySchema(4) == V::readable);

            // And the upper bound, which `< oldestReadable` alone would have missed: a session from
            // a later build must be refused rather than read with today's assumptions, which would
            // produce plausible wrong values instead of an obvious fallback.
            expect(classifySchema(5) == V::tooNew);
            expect(classifySchema(99) == V::tooNew);
        }
    }
};

static ProgramManagerTests programManagerTests;
