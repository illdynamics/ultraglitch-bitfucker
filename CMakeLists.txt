cmake_minimum_required(VERSION 3.22)

project(UltraGlitch VERSION 0.2.7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")
endif()

# Fetch JUCE using FetchContent — single source of truth, NO find_package
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.3
)

FetchContent_MakeAvailable(JUCE)

# ── Determine plugin formats per platform ────────────────────────────────────
if(APPLE)
    set(ULTRAGLITCH_FORMATS VST3 AU Standalone)
else()
    set(ULTRAGLITCH_FORMATS VST3 Standalone)
endif()

# ── Plugin target ────────────────────────────────────────────────────────────
juce_add_plugin(UltraGlitch
    COMPANY_NAME "UltraGlitch"
    BUNDLE_ID "com.ultraglitch.audio.ultraglitch"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE

    PLUGIN_MANUFACTURER_CODE UGlc
    PLUGIN_CODE UGBF

    FORMATS AU VST3 Standalone

    PRODUCT_NAME "UltraGlitch BitFucker"
)

# ── Source files ─────────────────────────────────────────────────────────────
target_sources(UltraGlitch PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Parameters/PluginParameters.cpp
    Source/DSP/EffectChain.cpp
    Source/DSP/EffectBase.cpp
    Source/DSP/Effects/BitCrusher.cpp
    Source/DSP/Effects/BufferStutter.cpp
    Source/DSP/Effects/PitchDrift.cpp
    Source/DSP/Effects/ReverseSlice.cpp
    Source/DSP/Effects/SliceRearrange.cpp
    Source/DSP/Effects/WeirdFlanger.cpp
    Source/DSP/Effects/ChaosController.cpp
    Source/GUI/MainPanel.cpp
    Source/GUI/LookAndFeel/GlitchLookAndFeel.cpp
    Source/GUI/Components/GlitchKnob.cpp
    Source/GUI/Components/PowerButton.cpp
    Source/GUI/Components/EffectModule.cpp
    Source/GUI/Components/ChaosButton.cpp
)

# ── Include directories ──────────────────────────────────────────────────────
target_include_directories(UltraGlitch PRIVATE
    Source
    Source/Common
    Source/DSP
    Source/DSP/Effects
    Source/Parameters
    Source/GUI
    Source/GUI/LookAndFeel
    Source/GUI/Components
)

# ── Link JUCE modules (juce_dsp needed for PitchDrift DelayLine) ─────────────
target_link_libraries(UltraGlitch
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

# ── Compile definitions ──────────────────────────────────────────────────────
target_compile_definitions(UltraGlitch PRIVATE
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

if(APPLE)
    target_compile_definitions(UltraGlitch PRIVATE
        JUCE_PLUGINHOST_AU=1
    )
    set_target_properties(UltraGlitch PROPERTIES
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
        XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT dwarf-with-dsym
    )
endif()
