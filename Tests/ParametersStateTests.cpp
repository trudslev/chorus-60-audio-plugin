#include "TestUtils.h"
#include "../Source/Parameters.h"
#include "../Source/DSP/FactoryPrograms.h"
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
        beginTest("Every factory Program value survives its parameter's mapping unchanged");
        {
            // The question the Rate widening raises: does any of the nine Programs load differently
            // now? FactoryPrograms stores ABSOLUTE values (0.55f means 0.55 Hz), applied through
            // AudioParameterFloat::operator=, which normalises against the parameter's own range.
            // So a stored value is preserved exactly as long as it lies inside that range - and
            // silently clamped if it does not.
            //
            // This asserts the round trip directly, per program and per field, rather than trusting
            // that the ranges happen to be wide enough. Widening a range can never clamp something
            // that already fitted; NARROWING one can, and this is what would catch it.
            DummyProcessor processor;
            juce::AudioProcessorValueTreeState apvts(processor, nullptr, "PARAMETERS",
                                                      createChorus60ParameterLayout());

            const auto roundTrips = [&] (const char* id, float value, const juce::String& where)
            {
                auto* p = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(id));
                expect(p != nullptr, juce::String(id) + " must exist");
                if (p == nullptr)
                    return;

                const auto& range = p->getNormalisableRange();
                const float back = range.convertFrom0to1(range.convertTo0to1(value));
                expectWithinAbsoluteError(back, value, 1.0e-3f,
                                           where + " " + id + " does not survive its range");
            };

            const char* rateIDs[3]   = {ParamIDs::rate1, ParamIDs::rate2, ParamIDs::rateB};
            const char* depthIDs[3]  = {ParamIDs::depth1, ParamIDs::depth2, ParamIDs::depthB};
            const char* centreIDs[3] = {ParamIDs::center1, ParamIDs::center2, ParamIDs::centerB};
            const char* decorrIDs[3] = {ParamIDs::decorr1, ParamIDs::decorr2, ParamIDs::decorrB};

            for (const auto& program : kFactoryPrograms)
            {
                const FactoryConfiguration* configs[3] =
                    {&program.configI, &program.configII, &program.configBoth};

                for (int i = 0; i < 3; ++i)
                {
                    const juce::String where(program.name);
                    roundTrips(rateIDs[i],   configs[i]->rateHz, where);
                    roundTrips(depthIDs[i],  configs[i]->depthPercent, where);
                    roundTrips(centreIDs[i], configs[i]->delayCentreMs, where);
                    roundTrips(decorrIDs[i], configs[i]->decorrelationPercent, where);
                }

                roundTrips(ParamIDs::drift,      program.driftPercent,      program.name);
                roundTrips(ParamIDs::saturation, program.saturationPercent, program.name);
                roundTrips(ParamIDs::noise,      program.noisePercent,      program.name);
                roundTrips(ParamIDs::mix,        program.mixPercent,        program.name);
                roundTrips(ParamIDs::trim,       program.trimDb,            program.name);
            }
        }

        beginTest("Defaults match Parameters.h's own values");
        {
            DummyProcessor proc;
            juce::AudioProcessorValueTreeState apvts(proc, nullptr, "PARAMETERS", createChorus60ParameterLayout());

            expect(*apvts.getRawParameterValue(ParamIDs::engine1) > 0.5f);
            expect(*apvts.getRawParameterValue(ParamIDs::engine2) < 0.5f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::rate1), 0.45f, 0.001f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::depth1), 38.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::rate2), 2.90f, 0.001f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::depth2), 64.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::center1), 5.6f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::decorr1), 52.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::drift), 22.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::saturation), 30.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::noise), 14.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::mix), 50.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvts.getRawParameterValue(ParamIDs::trim), 0.0f, 0.01f);

            // The defining asymmetry from design/BBD-TECHNICAL-NOTES-ADDENDUM.md: I and II are
            // stereo (the right channel's modulation inverted), I+II collapses to mono as the real
            // circuit does. If these ever come up equal, the correction has been undone.
            // The IMAGE switch: true = MONO. Section 11's defaults are STEREO / STEREO / MONO, the
            // last one because the real circuit's I+II mode is a mono BBD pair.
            expect(*apvts.getRawParameterValue(ParamIDs::image1) < 0.5f, "I should default to STEREO");
            expect(*apvts.getRawParameterValue(ParamIDs::image2) < 0.5f, "II should default to STEREO");
            expect(*apvts.getRawParameterValue(ParamIDs::imageB) > 0.5f, "I+II should default to MONO");

            // All three Rates share one range and one skew. This is not cosmetic tidying: the plate
            // prints ONE 0.05-16 Hz scale, used by whichever page is showing, so a narrower range on
            // I or II would put its pointer at the wrong printed numeral for every value.
            //
            // The skew is asserted too, because the printed marks were placed from it. Section 7.1's
            // five anchors come out of ((v-0.05)/15.95)^0.35 to within 0.09 degrees of arc.
            for (const auto* id : {ParamIDs::rate1, ParamIDs::rate2, ParamIDs::rateB})
            {
                auto* r = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(id));
                expect(r != nullptr, juce::String(id) + " must exist");
                if (r == nullptr)
                    continue;

                const auto range = r->getNormalisableRange();
                expectWithinAbsoluteError(range.start, 0.05f, 1.0e-6f, juce::String(id) + " start");
                expectWithinAbsoluteError(range.end, 16.0f, 1.0e-6f, juce::String(id) + " end");
                expectWithinAbsoluteError(range.skew, 0.35f, 1.0e-6f, juce::String(id) + " skew");

                // The printed marks, checked through the parameter's own mapping rather than a
                // restatement of the curve. 0.5 Hz sits at 28.7% of sweep, 2 Hz at 47.9%, 8 Hz at
                // 78.4% - section 7.1.
                expectWithinAbsoluteError(range.convertTo0to1(0.5f), 0.287f, 0.002f,
                                           juce::String(id) + " 0.5Hz mark");
                expectWithinAbsoluteError(range.convertTo0to1(2.0f), 0.479f, 0.002f,
                                           juce::String(id) + " 2Hz mark");
                expectWithinAbsoluteError(range.convertTo0to1(8.0f), 0.784f, 0.002f,
                                           juce::String(id) + " 8Hz mark");
            }
        }

        beginTest("Full round trip through state XML preserves values, including skewed parameters");
        {
            DummyProcessor procA;
            juce::AudioProcessorValueTreeState apvtsA(procA, nullptr, "PARAMETERS", createChorus60ParameterLayout());

            *dynamic_cast<juce::AudioParameterBool*>(apvtsA.getParameter(ParamIDs::engine2)) = true;
            *dynamic_cast<juce::AudioParameterFloat*>(apvtsA.getParameter(ParamIDs::rate1)) = 1.75f;
            *dynamic_cast<juce::AudioParameterFloat*>(apvtsA.getParameter(ParamIDs::center1)) = 12.5f;
            *dynamic_cast<juce::AudioParameterFloat*>(apvtsA.getParameter(ParamIDs::trim)) = -9.0f;

            auto state = apvtsA.copyState();
            std::unique_ptr<juce::XmlElement> xml(state.createXml());

            DummyProcessor procB;
            juce::AudioProcessorValueTreeState apvtsB(procB, nullptr, "PARAMETERS", createChorus60ParameterLayout());
            apvtsB.replaceState(juce::ValueTree::fromXml(*xml));

            expect(*apvtsB.getRawParameterValue(ParamIDs::engine2) > 0.5f);
            expectWithinAbsoluteError((float) *apvtsB.getRawParameterValue(ParamIDs::rate1), 1.75f, 0.001f);
            expectWithinAbsoluteError((float) *apvtsB.getRawParameterValue(ParamIDs::center1), 12.5f, 0.01f);
            expectWithinAbsoluteError((float) *apvtsB.getRawParameterValue(ParamIDs::trim), -9.0f, 0.01f);
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
            expectWithinAbsoluteError((float) *apvtsNew.getRawParameterValue(ParamIDs::drift), 22.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvtsNew.getRawParameterValue(ParamIDs::saturation), 30.0f, 0.01f);
            expectWithinAbsoluteError((float) *apvtsNew.getRawParameterValue(ParamIDs::trim), 0.0f, 0.01f);
        }
    }
};

static ParametersStateTests parametersStateTests;
