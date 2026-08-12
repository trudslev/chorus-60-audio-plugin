#include "../Source/Parameters.h"
#include "../Source/GUI/Chorus60Theme.h"

#include <nf/ParameterReadout.h>

#include <juce_audio_processors/juce_audio_processors.h>

namespace
{
    /** The smallest processor that can hold this casting's real parameter layout.

        The layout is the thing under test, so it has to be the shipping one - a hand-listed copy
        would assert against itself and pass while the panel printed something else.
    */
    class LayoutHost final : public juce::AudioProcessor
    {
    public:
        LayoutHost() : apvts (*this, nullptr, "PARAMETERS", createChorus60ParameterLayout()) {}

        juce::AudioProcessorValueTreeState apvts;

        const juce::String getName() const override { return "LayoutHost"; }
        void prepareToPlay (double, int) override {}
        void releaseResources() override {}
        void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram (int) override {}
        const juce::String getProgramName (int) override { return {}; }
        void changeProgramName (int, const juce::String&) override {}
        void getStateInformation (juce::MemoryBlock&) override {}
        void setStateInformation (const void*, int) override {}
    };
}

/**
    Every parameter must print acceptably in the LCD takeover.

    **This guards the defect CLASS, not the instances.** Extracting nf::describeParameter fixed the
    six castings as they stood; it does nothing about the next parameter somebody adds without an
    interval on its NormalisableRange and without a stringFromValueFunction. That is exactly how
    TapeRot came to print "DRIVE: 20.0000000" - its formatters were not deleted, they were never
    written, one parameter at a time, while the panel looked fine for every value that happened to
    be integral.

    Swept at several points across each parameter's range rather than at its default, because the
    default is the value most likely to be round - and a round value is precisely the one that hides
    this.
*/
class ReadoutConformanceTests final : public juce::UnitTest
{
public:
    ReadoutConformanceTests() : juce::UnitTest ("Readout conformance", "GUI") {}

    void runTest() override
    {
        beginTest ("Every parameter prints cleanly across its range");
        {
            LayoutHost host;

            // Deliberately not 0, 0.5, 1: those land on round physical values for a linear range,
            // which is where a missing formatter looks correct. The awkward fractions are the point.
            for (const float position : { 0.0f, 0.13f, 0.37f, 0.5f, 0.61f, 0.89f, 1.0f })
            {
                for (auto* raw : host.getParameters())
                {
                    auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*> (raw);

                    if (p == nullptr)
                        continue;

                    p->setValueNotifyingHost (position);

                    const auto defects = nf::readoutDefects (*p, Chorus60Theme::readoutFormat());

                    for (const auto& defect : defects)
                        expect (false, p->paramID + " at " + juce::String (position, 2)
                                     + ": " + defect);
                }
            }
        }

        beginTest ("The readout fits the LCD's character budget at every position");
        {
            // The takeover shares the cell with the Program name, and the name cap was computed
            // from that cell. A readout that overruns it is silently clipped, which reads as a
            // truncated unit rather than as a layout problem.
            //
            // The guard is the 16px step-down budget rather than the 19px one, because the readout
            // is what the step-down exists to absorb.
            LayoutHost host;
            const int budget = Chorus60Theme::Layout::lcdCharacterBudget + 8;

            for (const float position : { 0.0f, 0.37f, 1.0f })
                for (auto* raw : host.getParameters())
                    if (auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*> (raw))
                    {
                        p->setValueNotifyingHost (position);

                        const auto text = nf::describeParameter (*p, Chorus60Theme::readoutFormat());
                        expect (text.length() <= budget,
                                p->paramID + " prints " + juce::String (text.length())
                                    + " characters: \"" + text + "\"");
                    }
        }
    }
};

static ReadoutConformanceTests readoutConformanceTests;
