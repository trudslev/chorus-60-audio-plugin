#pragma once

#include "Parameters.h"
#include "DSP/BBDDelayLine.h"
#include "DSP/CharacterStage.h"
#include "DSP/ModulationEngine.h"
#include "DSP/OutputMixStage.h"
#include "DSP/ProgramManager.h"
#include "DSP/StereoDecorrelationStage.h"
#include <juce_audio_processors/juce_audio_processors.h>

class Chorus60AudioProcessor final : public juce::AudioProcessor
{
public:
    Chorus60AudioProcessor();
    ~Chorus60AudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    // Real tail here is genuinely short - a BBD chorus's delay line never exceeds ~17ms (15ms max
    // Delay Center + a couple ms excursion), unlike a reverb's open-ended decay.
    double getTailLengthSeconds() const override { return 0.05; }

    int getNumPrograms() override { return programManager.getNumPrograms(); }
    int getCurrentProgram() override { return programManager.getCurrentProgram(); }
    void setCurrentProgram(int index) override { programManager.requestProgramChange(index); }
    const juce::String getProgramName(int index) override { return programManager.getProgramName(index); }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool isFactoryProgram(int index) const noexcept { return programManager.isFactoryProgram(index); }
    bool isCurrentProgramModified() const { return programManager.isModifiedFromLoadedProgram(); }
    void saveNewUserProgram(const juce::String& name) { programManager.saveNewUserProgram(name); }
    void deleteUserProgram(int index) { programManager.deleteUserProgram(index); }

    juce::AudioProcessorValueTreeState apvts;

    // GUI-facing derived-display state, mirroring both siblings' poll-based pattern - the scope
    // polls these once per repaint and accumulates its own scrolling history locally.
    float getModulationOffsetMs() const noexcept { return modulationDisplayValue.load(std::memory_order_relaxed); }
    float getDriftOffsetMs() const noexcept { return driftDisplayValue.load(std::memory_order_relaxed); }
    float getInputMeterDb() const noexcept { return inputMeterDb.load(std::memory_order_relaxed); }
    float getOutputMeterDb() const noexcept { return outputMeterDb.load(std::memory_order_relaxed); }

    // Which of the three parameter sets the engine latches currently select. Public because the
    // GUI needs exactly the same answer the audio thread uses - the scope has to follow the engaged
    // configuration's rate, depth and centre, and the paged MOD ENGINE box shows that page. Having
    // one implementation rather than two is the point: a second copy of this selection in the GUI
    // is precisely the kind of duplicated fact that drifts.
    enum class Configuration { one, two, both, bypassed };

    // The resolved values for the block being processed.
    struct ActiveConfiguration
    {
        Configuration which = Configuration::bypassed;
        float rateHz = 0.0f;
        float depthPercent = 0.0f;
        float centreMs = 0.0f;
        float decorrelationPercent = 0.0f;
        bool mono = false;
        bool engaged = false; // false when neither latch is on - the panel's OFF/bypass state
    };

    ActiveConfiguration resolveActiveConfiguration() const;

private:
    ProgramManager programManager;

    // One configuration's five parameters. Three of these exist - I, II and I+II - and exactly one
    // is selected per block by the engine latches; see resolveActiveConfiguration().
    struct ConfigurationParams
    {
        std::atomic<float>* rate = nullptr;
        std::atomic<float>* depth = nullptr;
        std::atomic<float>* centre = nullptr;
        std::atomic<float>* decorrelation = nullptr;
        std::atomic<float>* mono = nullptr;
    };

    std::atomic<float>* engine1Param = nullptr;
    std::atomic<float>* engine2Param = nullptr;

    ConfigurationParams configI;
    ConfigurationParams configII;
    ConfigurationParams configBoth;

    std::atomic<float>* driftParam = nullptr;
    std::atomic<float>* saturationParam = nullptr;
    std::atomic<float>* noiseParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* trimParam = nullptr;

    // Singular, deliberately. The real circuit has one LFO; I+II is a third configuration of it,
    // not I and II summed. See design/BBD-TECHNICAL-NOTES-ADDENDUM.md.
    ModulationEngine modulationEngine;
    BBDDelayLine bbdDelayLine;
    StereoDecorrelationStage stereoDecorrelationStage;
    CharacterStage characterStage;
    OutputMixStage outputMixStage;

    juce::AudioBuffer<float> dryBuffer;

    std::atomic<float> modulationDisplayValue { 0.0f };
    std::atomic<float> driftDisplayValue { 0.0f };
    std::atomic<float> inputMeterDb { -100.0f };
    std::atomic<float> outputMeterDb { -100.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Chorus60AudioProcessor)
};
