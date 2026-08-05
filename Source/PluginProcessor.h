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

private:
    ProgramManager programManager;

    std::atomic<float>* engine1Param = nullptr;
    std::atomic<float>* engine2Param = nullptr;
    std::atomic<float>* rate1Param = nullptr;
    std::atomic<float>* depth1Param = nullptr;
    std::atomic<float>* rate2Param = nullptr;
    std::atomic<float>* depth2Param = nullptr;
    std::atomic<float>* delayCenterParam = nullptr;
    std::atomic<float>* decorrelationParam = nullptr;
    std::atomic<float>* driftParam = nullptr;
    std::atomic<float>* saturationParam = nullptr;
    std::atomic<float>* noiseParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* trimParam = nullptr;

    ModulationEngine modulationEngine1;
    ModulationEngine modulationEngine2;
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
