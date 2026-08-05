juce_add_console_app(Chorus60Tests PRODUCT_NAME "Chorus60Tests")

target_sources(Chorus60Tests PRIVATE
    TestMain.cpp
    ModulationEngineTests.cpp
    BBDDelayLineTests.cpp
    StereoDecorrelationStageTests.cpp
    CharacterStageTests.cpp
    OutputMixStageTests.cpp
    ParametersStateTests.cpp
    ProgramManagerTests.cpp
    FactoryProgramsTests.cpp
    CPUCheckTests.cpp
    ../Source/DSP/ModulationEngine.cpp
    ../Source/DSP/BBDDelayLine.cpp
    ../Source/DSP/StereoDecorrelationStage.cpp
    ../Source/DSP/CharacterStage.cpp
    ../Source/DSP/OutputMixStage.cpp
    ../Source/DSP/ProgramManager.cpp)

# NF_COMPANY_NAME/NF_PRODUCT_NAME come from the parent CMakeLists and must match what the plugin
# target gets - ProgramManager.cpp is compiled into both, and builds the user-Programs path from
# them. Inherited from the parent scope rather than restated here, so there is nothing to re-sync.
target_compile_definitions(Chorus60Tests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    NF_COMPANY_NAME="${CHORUS60_COMPANY_NAME}"
    NF_PRODUCT_NAME="${CHORUS60_PRODUCT_NAME}")

target_link_libraries(Chorus60Tests PRIVATE
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
