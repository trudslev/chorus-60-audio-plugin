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

project(Chorus60 VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.14
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# Runtime GUI assets only.
#
# Revision 2 inverted the plate's role. It used to be bare material with every glyph drawn in code;
# it now BAKES the printed scales, every tick ring, every numeral and unit, the static labels, the
# wordmark and the group headings - see design/CHORUS60-BUILD-HANDOFF.md section 1, which is the
# asset contract. Only nine things are drawn at runtime, and the code that used to draw the rest is
# deleted rather than adapted: drawing a baked string again double-prints it at a one-pixel offset.
#
# The four chorus60-page-*@2x renders are full composites - plate plus every runtime element - and
# are pixel-matching acceptance targets, never runtime assets or a source to slice. Same for the
# hardware photo and gatecrasher-panel-reference.png.
#
# LibrestileExtBold is gone: the wordmark is baked now, so nothing draws it live. It stays in
# design/assets/ so the plate can be re-exported.
#
# Knob filmstrips: the @1x strips, and they are final. There are no @2x sheets to embed - earlier
# revisions shipped upsampled placeholders and the designers REMOVED them rather than leave a file
# named @2x sitting in an assets folder where it would get wired up on sight and ship a soft knob
# under a retina label.
#
# The knob is drawn by a parametric generator taking a scale factor, so any diameter is reachable and
# producing the real Ø168/Ø136 sheets is a config change rather than new artwork; handoff section 4
# has the run parameters. KnobFilmstripComponent takes the sheet geometry as data - frame pitch,
# column count and cap-to-pitch ratio - so they drop in with no drawing-code change. Tracked as a
# release blocker in prompts/PROMPTS.md.
#
# Fonts: Barlow Condensed SemiBold (600 - control labels, captions, scope status) and Bold (700 -
# group headings, engine button letters) per spec section 3, plus Share Tech Mono Regular for
# everything inside a display (LCD name, parameter readout, IN/OUT, footer).
juce_add_binary_data(Chorus60BinaryData
    SOURCES
        design/assets/chorus60-background-plate@2x.png
        design/assets/knob_mod_84px_128f.png
        design/assets/knob_global_68px_128f.png
        design/assets/controls/button-i@2x.png
        design/assets/controls/button-ii@2x.png
        design/assets/controls/button-off@2x.png
        design/assets/controls/lamp-on@2x.png
        design/assets/controls/lamp-off@2x.png
        design/assets/controls/switch-track@2x.png
        design/assets/controls/switch-thumb@2x.png
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

# Single source of truth for the two identity strings that appear BOTH in the bundle metadata and
# in ProgramManager's user-Programs directory path. ProgramManager.cpp can't read JUCE's
# JucePlugin_* macros - they only exist in the plugin target's generated header, and that file is
# also compiled into the Tests console app - so it used to carry a hand-synced copy marked "keep in
# sync". It didn't stay in sync: COMPANY_NAME became "Neon Foundry" while the copy still said
# "Tanis", quietly pointing saved Programs at the old directory. CMake now hands both targets the
# same values, so drift is a build error instead of silent data loss.
set(CHORUS60_COMPANY_NAME "Neon Foundry")
set(CHORUS60_PRODUCT_NAME "CHORUS-60")

juce_add_plugin(Chorus60
    PRODUCT_NAME "${CHORUS60_PRODUCT_NAME}"
    COMPANY_NAME "${CHORUS60_COMPANY_NAME}"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/design/assets/icon/chorus60-icon-1024.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/design/assets/icon/chorus60-icon-256.png"
    BUNDLE_ID "com.neonfoundry.chorus60"
    # Without this the Standalone cannot get audio INPUT on macOS, and fails silently doing it.
    # An app with no NSMicrophoneUsageDescription is never granted capture access - the OS does not
    # prompt and does not error, the input stream simply delivers zeros. On an effect that reads as
    # dead metering: IN sits at -INF and OUT follows, because there is nothing to process. The
    # meters are wired correctly; they have nothing to show.
    #
    # Plugin formats are unaffected - the host owns the input there - so this only shows up when
    # someone opens the Standalone to listen.
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "CHORUS-60 needs audio input so the Standalone can process what you play into it."
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
    Source/GUI/ImageSwitch.cpp
    Source/GUI/DimmableGroup.cpp
    Source/GUI/ModScope.cpp
    Source/GUI/EngineButtonComponent.cpp
    Source/GUI/ProgramHeader.cpp
    Source/GUI/Chorus60EditorContent.cpp)

target_compile_definitions(Chorus60 PUBLIC
    NF_VERSION_SHORT="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    NF_COMPANY_NAME="${CHORUS60_COMPANY_NAME}"
    NF_PRODUCT_NAME="${CHORUS60_PRODUCT_NAME}")

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
