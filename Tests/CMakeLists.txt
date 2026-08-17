juce_add_console_app(Chorus60Tests PRODUCT_NAME "Chorus60Tests")

target_sources(Chorus60Tests PRIVATE
    RealtimeSafetyTests.cpp
    NumericalRobustnessTests.cpp
    InvarianceTests.cpp
    AutomationLatencyTests.cpp
    ChannelLayoutTests.cpp
    PendingProgramLockTests.cpp
    MeterReadoutBudgetTests.cpp
    TestMain.cpp
    ReadoutConformanceTests.cpp
    MenuMetricsTests.cpp
    ModulationEngineTests.cpp
    BBDDelayLineTests.cpp
    StereoDecorrelationStageTests.cpp
    CharacterStageTests.cpp
    OutputMixStageTests.cpp
    ParametersStateTests.cpp
    ProgramManagerTests.cpp
    FactoryProgramsTests.cpp
    CPUCheckTests.cpp
    DisplayBudgetTests.cpp
    KnobRenderTests.cpp
    PrintedScaleTests.cpp
    ../Source/GUI/KnobScaleRing.cpp
    ../Source/DSP/ModulationEngine.cpp
    ../Source/DSP/BBDDelayLine.cpp
    ../Source/DSP/StereoDecorrelationStage.cpp
    ../Source/DSP/CharacterStage.cpp
    ../Source/DSP/OutputMixStage.cpp
    ../Source/DSP/ProgramManager.cpp
    # **The REAL processor and the REAL editor, not stand-ins.** Everything above this line is what
    # core does not own at the DSP layer; everything below it is the wiring and the panel, which no
    # test could reach while this target compiled DSP alone.
    #
    # That gap was not theoretical. Reflect-84 shipped the stale-replay guard with ZERO call sites
    # for its disarm, and no suite could have seen it: the defect lived in an editor .cpp that was
    # not compiled into anything a test could run.
    ../Source/PluginProcessor.cpp
    ../Source/PluginEditor.cpp
    ../Source/GUI/Chorus60EditorContent.cpp
    ../Source/GUI/ProgramHeader.cpp
    ../Source/GUI/Chorus60PanelBackground.cpp
    ../Source/GUI/Chorus60LookAndFeel.cpp
    ../Source/GUI/KnobFilmstripComponent.cpp
    ../Source/GUI/EngineButtonComponent.cpp
    ../Source/GUI/ImageSwitch.cpp
    ../Source/GUI/DimmableGroup.cpp
    ../Source/GUI/ModScope.cpp)

# NF_COMPANY_NAME/NF_PRODUCT_NAME come from the parent CMakeLists and must match what the plugin
# target gets - ProgramManager.cpp is compiled into both, and builds the user-Programs path from
# them. Inherited from the parent scope rather than restated here, so there is nothing to re-sync.
#
# The JucePlugin_* pair and NF_VERSION_SHORT arrived with the processor and panel sources: a console
# app gets none of the plugin target's generated definitions, and PluginProcessor uses
# JucePlugin_Name for getName() while PanelBackground prints the version stamp. Inherited from the
# same parent variables the plugin target uses, so there is nothing to re-sync — a hand-synced copy
# in CHORUS-60 once drifted to a dead company name and quietly pointed saved Programs at a directory
# nothing reads.
target_compile_definitions(Chorus60Tests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    NF_COMPANY_NAME="${CHORUS60_COMPANY_NAME}"
    NF_PRODUCT_NAME="${CHORUS60_PRODUCT_NAME}"
    JucePlugin_Name="${CHORUS60_PRODUCT_NAME}"
    JucePlugin_Manufacturer="${CHORUS60_COMPANY_NAME}"
    NF_VERSION_SHORT="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

target_link_libraries(Chorus60Tests PRIVATE
    # The bug sweep's measurement drivers. TEST target only — never the plugin, which must not
    # carry an operator new override. See nf/testing/ProcessorHarness.h.
    nf::testing
    # ProgramManager.cpp is compiled into both targets and calls into core, so both link it.
    nf::core
    # Chorus60BinaryData, because ReadoutConformanceTests includes Chorus60Theme.h to read
    # readoutFormat() - the SHIPPING format, not a copy of it - and the theme pulls in BinaryData.h
    # for the embedded faces. Linking the real generated header rather than stubbing it is the
    # point, and it is what Fifth Member's and TapeRot's test targets already do.
    Chorus60BinaryData
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
