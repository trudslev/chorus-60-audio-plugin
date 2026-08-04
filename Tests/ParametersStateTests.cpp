#include "TestUtils.h"
#include "../Source/Parameters.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace
{
    // Minimal throwaway AudioProcessor just to host an APVTS in tests, avoiding any dependency on
    // the real plugin target's JucePlugin_* macros (which PluginProcessor.cpp requires and which
    // aren't available in the plain console-app Tests target).
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

class ParametersStateTests final : public juce::UnitTest
{
public:
    ParametersStateTests() : juce::UnitTest("ParametersState", "DSP") {}

    void runTest() override
    {
        beginTest("Defaults match Parameters.h's own values");
        {
            DummyProcessor proc;
            juce::AudioProcessorValueTreeState apvts(proc, nullptr, "PARAMETERS", createChorus60ParameterLayout());

            expect(*apvts.getRawParameterValue(ParamIDs::engine1) > 0.5f);
            expect(*apvts.getRawParameterValue(ParamIDs::engine2) < 0.5f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::rate1), 0.55f, 0.001f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::depth1), 25.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::rate2), 1.0f, 0.001f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::depth2), 55.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::delayCenter), 8.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::decorrelation), 70.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::drift), 40.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::saturation), 15.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::noise), 20.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::mix), 50.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::trim), 0.0f, 0.01f);
        }

        beginTest("Full round trip through state XML preserves values, including skewed parameters");
        {
            DummyProcessor procA;
            juce::AudioProcessorValueTreeState apvtsA(procA, nullptr, "PARAMETERS", createChorus60ParameterLayout());

            *dynamic_cast<juce::AudioParameterBool*>(apvtsA.getParameter(ParamIDs::engine2)) = true;
            *dynamic_cast<juce::AudioParameterFloat*>(apvtsA.getParameter(ParamIDs::rate1)) = 1.75f;
            *dynamic_cast<juce::AudioParameterFloat*>(apvtsA.getParameter(ParamIDs::delayCenter)) = 12.5f;
            *dynamic_cast<juce::AudioParameterFloat*>(apvtsA.getParameter(ParamIDs::trim)) = -18.0f;

            auto state = apvtsA.copyState();
            std::unique_ptr<juce::XmlElement> xml(state.createXml());

            DummyProcessor procB;
            juce::AudioProcessorValueTreeState apvtsB(procB, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            apvtsB.replaceState(juce::ValueTree::fromXml(*xml));

            expect(*apvtsB.getRawParameterValue(ParamIDs::engine2) > 0.5f);
            expectWithinAbsoluteError((float) *apvtsB.getRawParameterValue(ParamIDs::rate1), 1.75f, 0.001f);
            expectWithinAbsoluteError((float) *apvtsB.getRawParameterValue(ParamIDs::delayCenter), 12.5f, 0.01f);
            expectWithinAbsoluteError((float) *apvtsB.getRawParameterValue(ParamIDs::trim), -18.0f, 0.01f);
        }

        beginTest("Missing parameters in an old saved session fall back to defaults without crashing");
        {
            DummyProcessor procOld;
            juce::AudioProcessorValueTreeState apvtsOld(procOld, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            *dynamic_cast<juce::AudioParameterFloat*>(apvtsOld.getParameter(ParamIDs::rate1)) = 1.9f;

            auto state = apvtsOld.copyState();
            std::unique_ptr<juce::XmlElement> xml(state.createXml());

            const juce::StringArray idsToStrip{ ParamIDs::drift, ParamIDs::saturation, ParamIDs::trim };
            for (auto& id : idsToStrip)
                for (int i = xml->getNumChildElements(); --i >= 0;)
                {
                    auto* child = xml->getChildElement(i);
                    if (child->getStringAttribute("id") == id)
                        xml->removeChildElement(child, true);
                }

            DummyProcessor procNew;
            juce::AudioProcessorValueTreeState apvtsNew(procNew, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            apvtsNew.replaceState(juce::ValueTree::fromXml(*xml));

            expectWithinAbsoluteError((float) *apvtsNew.getRawParameterValue(ParamIDs::rate1), 1.9f, 0.001f);
            expectWithinAbsoluteError((float) *apvtsNew.getRawParameterValue(ParamIDs::drift), 40.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvtsNew.getRawParameterValue(ParamIDs::saturation), 15.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvtsNew.getRawParameterValue(ParamIDs::trim), 0.0f, 0.01f);
        }
    }
};

static ParametersStateTests parametersStateTests;
