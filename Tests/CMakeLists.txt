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

target_compile_definitions(Chorus60Tests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0)

target_link_libraries(Chorus60Tests PRIVATE
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
