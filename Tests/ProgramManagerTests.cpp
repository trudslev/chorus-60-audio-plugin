#include "TestUtils.h"
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
        beginTest("isFactoryProgram is true for [0, kNumFactoryPrograms) and false outside it");
        {
            DummyProcessor proc;
            juce::AudioProcessorValueTreeState apvts(proc, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            ProgramManager manager(apvts);

            expect(! manager.isFactoryProgram(-1));
            expect(manager.isFactoryProgram(0));
            expect(manager.isFactoryProgram(kNumFactoryPrograms - 1));
            expect(! manager.isFactoryProgram(kNumFactoryPrograms));
        }

        beginTest("getProgramName returns the factory table's names before any user programs exist");
        {
            DummyProcessor proc;
            juce::AudioProcessorValueTreeState apvts(proc, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            ProgramManager manager(apvts);

            for (int i = 0; i < kNumFactoryPrograms; ++i)
                expectEquals(manager.getProgramName(i), juce::String(kFactoryPrograms[(size_t) i].name));

            expectEquals(manager.getNumPrograms(), kNumFactoryPrograms);
        }

        beginTest("getCurrentProgram() defaults to defaultFactoryProgramIndex before initialise()");
        {
            DummyProcessor proc;
            juce::AudioProcessorValueTreeState apvts(proc, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            ProgramManager manager(apvts);

            expectEquals(manager.getCurrentProgram(), defaultFactoryProgramIndex);
        }

        beginTest("setCurrentProgramIndexWithoutApplying updates getCurrentProgram() without touching APVTS");
        {
            DummyProcessor proc;
            juce::AudioProcessorValueTreeState apvts(proc, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            ProgramManager manager(apvts);

            const float rate1Before = *apvts.getRawParameterValue(ParamIDs::rate1);
            manager.setCurrentProgramIndexWithoutApplying(2);

            expectEquals(manager.getCurrentProgram(), 2);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::rate1), rate1Before, 1.0e-6f);
        }
    }
};

static ProgramManagerTests programManagerTests;
