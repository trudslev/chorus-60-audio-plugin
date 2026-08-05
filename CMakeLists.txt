cmake_minimum_required(VERSION 3.24)
# LANGUAGES needs C as well as CXX: JUCE/VST3-SDK/other fetched third-party deps bundle some .c
# sources, and Xcode/Visual Studio tolerate that without an explicit enable_language(C), but
# Linux's single-config Makefiles/Ninja generator does not - it fails at CMake's generate step
# with "CMAKE_C_COMPILE_OBJECT not set" if C was never enabled at the top level.
# These MUST come before project(): project() creates these cache entries itself, after which a
# plain set(... CACHE ...) is a silent no-op. Sitting below project() they had never taken effect -
# builds inherited the host architecture and the default SDK deployment target instead, which only
# surfaced when a CI-published .pkg turned out to contain an arm64-only binary.
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal: Apple Silicon + Intel")
endif()

project(Chorus60 VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.14
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# Runtime GUI assets only - the two alternate panel-state screenshots
# (chorus60-panel@2x.png / -both-engines@2x.png), the hardware reference photo, and the
# gatecrasher-panel-reference.png are pixel-matching/authority targets for
# design/CHORUS60-GUI-SPEC.md, not loaded at runtime, so they're deliberately excluded here -
# chorus60-panel-bypass@2x.png (both engines off) is the one used as the live static background.
# LibrestileExtBold is embedded despite the spec allowing either baking or live drawing (section 8)
# because the wordmark treatment here is flat/clean, not distressed, so drawing it live from the
# embedded font is the simpler equally-valid choice - no PNG needed. Barlow Condensed SemiBold
# (600, labels) / Bold (700, group/lamp text) and Share Tech Mono Regular (numeric/LED readouts)
# match Gatecrasher's exact treatment (design/CLAUDE.md: "same as Gatecrasher").
juce_add_binary_data(Chorus60BinaryData
    SOURCES
        design/assets/chorus60-background-plate@2x.png
        design/assets/knob_large_128px_128f.png
        design/assets/knob_small_128px_128f.png
        design/assets/LibrestileExtBold.ttf
        design/assets/BarlowCondensed-SemiBold.ttf
        design/assets/BarlowCondensed-Bold.ttf
        design/assets/ShareTechMono-Regular.ttf)

# AU is Apple-only; Windows/Linux only ever build VST3 + Standalone. VST3_COPY_DIR/AU_COPY_DIR are
# only set on Apple - leaving them unset elsewhere lets JUCE apply its own correct per-OS default
# rather than a macOS path built on a non-guaranteed $ENV{HOME}.
if(APPLE)
    set(CHORUS60_FORMATS AU VST3 Standalone)
    set(CHORUS60_EXTRA_ARGS
        VST3_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3"
        AU_COPY_DIR   "$ENV{HOME}/Library/Audio/Plug-Ins/Components")
else()
    set(CHORUS60_FORMATS VST3 Standalone)
    set(CHORUS60_EXTRA_ARGS "")
endif()

juce_add_plugin(Chorus60
    PRODUCT_NAME "Chorus-60"
    COMPANY_NAME "Neon Foundry"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/design/assets/icon/chorus60-icon-1024.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/design/assets/icon/chorus60-icon-256.png"
    BUNDLE_ID "com.neonfoundry.chorus60"
    PLUGIN_MANUFACTURER_CODE Nfdy
    PLUGIN_CODE Ch60
    FORMATS ${CHORUS60_FORMATS}
    VST3_CATEGORIES Fx Modulation
    AU_MAIN_TYPE kAudioUnitType_Effect
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    ${CHORUS60_EXTRA_ARGS})

target_sources(Chorus60 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/ModulationEngine.cpp
    Source/DSP/BBDDelayLine.cpp
    Source/DSP/StereoDecorrelationStage.cpp
    Source/DSP/CharacterStage.cpp
    Source/DSP/OutputMixStage.cpp
    Source/DSP/ProgramManager.cpp
    Source/GUI/Chorus60LookAndFeel.cpp
    Source/GUI/Chorus60PanelBackground.cpp
    Source/GUI/KnobFilmstripComponent.cpp
    Source/GUI/ModScope.cpp
    Source/GUI/EngineButtonComponent.cpp
    Source/GUI/ProgramHeader.cpp
    Source/GUI/WordmarkComponent.cpp
    Source/GUI/PanelChrome.cpp
    Source/GUI/Chorus60EditorContent.cpp)

target_compile_definitions(Chorus60 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Chorus60
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        Chorus60BinaryData
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

add_subdirectory(Tests)
