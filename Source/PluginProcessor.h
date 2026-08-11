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

    //==============================================================================
    /** **The host adapter - the ONLY place a Program is addressed by position.**

        **The list is the Factory bank and nothing else** - not INIT, not User Programs. That is a
        conformance requirement: juce_AudioProcessor.h documents getNumPrograms as "The value
        returned must be valid as soon as this object is created, and must not change over its
        lifetime", and a count including User Programs changed the moment one was saved.

        Before anyone makes the count dynamic again: JUCE's VST3 wrapper builds the automatable
        Program parameter ONCE in its constructor from this value, so a Program saved afterwards was
        unreachable from the host. That was the API keeping its documented promise, not a bug.

        Excluding INIT too means host index n IS Factory Program n+1.

        **Accepted divergence.** getCurrentProgram answers 0 while a User Program is loaded, so a
        host's menu shows a Factory name while the panel shows the user's Program. Sound and panel
        are both correct; only the host's own menu is wrong. */
    int getNumPrograms() override { return programManager.getNumPrograms(); }
    int getCurrentProgram() override { return programManager.getCurrentFactoryPosition(); }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override { return programManager.getProgramName(index); }
    /** Deliberately a no-op: with Factory-only exposure nothing on the host's list can be renamed -
        Factory names are fixed and User Programs are not exposed. Implementing it would be a back
        door into the Factory bank, which is what the permanent slugs exist to prevent. */
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    /** The Program model. The panel talks to this directly, in ProgramIds. */
    ProgramManager& getProgramManager() noexcept { return programManager; }

    /** Clears the stale-replay guard. Called from the editor when a change is USER-originated. */
    void noteUserEdit() noexcept { justRestoredState.store(false, std::memory_order_relaxed); }
    bool isCurrentProgramModified() const { return programManager.isModifiedFromLoadedProgram(); }
    void saveNewUserProgram(const juce::String& name) { programManager.saveNewUserProgram(name); }
    void deleteUserProgram(const ProgramId& id) { programManager.deleteUserProgram(id); }

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
        // The IMAGE switch's resolved value: true collapses the modulation to mono. Named for the
        // value rather than the control, because that is what the DSP below acts on.
        bool mono = false;
        bool engaged = false; // false when neither latch is on - the panel's OFF/bypass state
    };

    ActiveConfiguration resolveActiveConfiguration() const;

private:
    /** **Guards a host replaying a stale program index over a just-restored session.** Armed by
        setStateInformation, disarmed by the first setCurrentProgram (itself ignored only when it
        matches what getCurrentProgram reports) or the first USER-originated edit. Automation must
        not disarm it: a host may write automation on load before replaying. */
    std::atomic<bool> justRestoredState { false };

    ProgramManager programManager;

    // One configuration's five parameters. Three of these exist - I, II and I+II - and exactly one
    // is selected per block by the engine latches; see resolveActiveConfiguration().
    struct ConfigurationParams
    {
        std::atomic<float>* rate = nullptr;
        std::atomic<float>* depth = nullptr;
        std::atomic<float>* centre = nullptr;
        std::atomic<float>* decorrelation = nullptr;
        std::atomic<float>* image = nullptr;
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
