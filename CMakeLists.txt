cmake_minimum_required(VERSION 3.22)

project(UltraGlitch VERSION 0.4.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── macOS: Universal Binary (arm64 + x86_64) ────────────────────────────────
# Produces a single fat binary that works natively on Apple Silicon AND
# under Rosetta on Intel. macOS automatically loads the correct architecture
# slice at runtime — no duplicate plugin entries, no conflicting bundle IDs.
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build universal binary for Apple Silicon + Intel" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum macOS deployment target" FORCE)

    # JUCE contains Objective-C/Objective-C++ code that still uses manual retain/release in places.
    # ARC breaks compilation (especially with the Xcode generator / CI). Keep ARC disabled.
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "NO")
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

# ── COPY_PLUGIN_AFTER_BUILD: safe per-platform ───────────────────────────────
# macOS: copies to ~/Library/Audio/Plug-Ins/ (user-writable, no admin needed)
# Windows: VST3 install path is under C:\Program Files\ (requires admin,
#          breaks CI/non-admin shells). Disabled by default; use the install
#          script or manually copy from the artefacts directory instead.
# Linux: similar concern to Windows, disabled by default.
if(APPLE)
    set(ULTRAGLITCH_COPY_AFTER_BUILD TRUE)
else()
    set(ULTRAGLITCH_COPY_AFTER_BUILD FALSE)
endif()

# ── Assets (BinaryData) ──────────────────────────────────────────────────────
# Put your chopped background image in your repo at: Source/Assets/ugbf1.png
# (or rename it, but then update the filename here)
juce_add_binary_data(UltraGlitchBinaryData
    SOURCES
        Source/Assets/ugbf1.png
)

# ── Plugin target ────────────────────────────────────────────────────────────
juce_add_plugin(UltraGlitch
    COMPANY_NAME "UltraGlitch"
    BUNDLE_ID "com.ultraglitch.audio.ultraglitch"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${ULTRAGLITCH_COPY_AFTER_BUILD}

    PLUGIN_MANUFACTURER_CODE UGlc
    PLUGIN_CODE UGBF

    FORMATS ${ULTRAGLITCH_FORMATS}

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
        UltraGlitchBinaryData
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
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO
        XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT dwarf-with-dsym
    )
endif()
