# UltraGlitch BitFucker: Technical Documentation

## 1. Project Overview

UltraGlitch BitFucker is a cross-platform audio effect plugin designed for creative glitch production. It provides a suite of destructive/creative effects popular in electronic music, IDM, and sound design, built using the JUCE C++ framework.

**Key Features:**
*   **BitCrusher:** Bit depth reduction and sample rate decimation.
*   **Buffer Stutter:** Rhythmic buffer repeat/stutter effect.
*   **Pitch Drift:** Continuous subtle or extreme pitch modulation.
*   **Reverse Slice:** Periodic audio reversal.
*   **Slice Rearrange:** Beat-synchronized slice shuffling.
*   **Weird Flanger:** Unconventional flanging/phasing with feedback.
*   **CHAOS MODE:** Randomizes all effect parameters continuously.

Each effect has independent enable/disable, mix (dry/wet), and effect-specific parameters. A global output gain control prevents clipping.

## 2. Architecture Diagram

The plugin follows a standard JUCE plugin architecture, augmented with a custom EffectChain and PluginParameters system.

```mermaid
graph TD
    A[DAW] --> B(UltraGlitchAudioProcessor)
    B --> C(PluginParameters)
    B --> D(EffectChain)
    B -- UI --> E(UltraGlitchAudioProcessorEditor)
    E --> F(MainPanel)
    C --> D
    C --> F
    F --> G{EffectModule}
    F --> H(ChaosButton)
    F --> I(GlitchKnob)
    G --> J(PowerButton)
    G --> K(GlitchKnob)
    D --> L{EffectBase (BitCrusher, BufferStutter, ...)}
    L -- Parameters --> C
    D -- Parameter Updates --> L
    D -- Audio Processing --> L
    B --> B_LF[GlitchLookAndFeel]
    F --> B_LF
    G --> B_LF
    H --> B_LF
    I --> B_LF
    J --> B_LF

    subgraph Core Plugin Components
        B
        C
        D
    end

    subgraph GUI Components
        E
        F
        G
        H
        I
        J
    end

    subgraph DSP Effects
        L
    end

    style B fill:#f9f,stroke:#333,stroke-width:2px
    style C fill:#ccf,stroke:#333,stroke-width:2px
    style D fill:#cfc,stroke:#333,stroke-width:2px
    style E fill:#fcf,stroke:#333,stroke-width:2px
    style F fill:#ffc,stroke:#333,stroke-width:2px
    style G fill:#fcc,stroke:#333,stroke-width:2px
    style H fill:#fce,stroke:#333,stroke-width:2px
    style I fill:#cef,stroke:#333,stroke-width:2px
    style J fill:#efc,stroke:#333,stroke-width:2px
    style K fill:#ddf,stroke:#333,stroke-width:2px
    style L fill:#eee,stroke:#333,stroke-width:2px
```

**Component Breakdown:**

*   **UltraGlitchAudioProcessor (`Source/PluginProcessor.h/cpp`):** The main audio processing unit. It inherits from `juce::AudioProcessor`, owns an instance of `PluginParameters` and `EffectChain`, and is responsible for overall audio flow and state management. It delegates parameter handling to `PluginParameters` and DSP to `EffectChain`.
*   **PluginParameters (`Source/Parameters/PluginParameters.h/cpp`):** Manages all plugin parameters using `juce::AudioProcessorValueTreeState`. It defines all parameters, their ranges, default values, and provides thread-safe access to them. It also handles parameter change notifications and state persistence.
*   **EffectChain (`Source/DSP/EffectChain.h/cpp`):** Orchestrates the DSP effects. It owns a collection of `EffectBase` derived objects, manages their processing order, and routes parameter updates from `PluginParameters` to the individual effects. It also performs global dry/wet mixing (or output gain application).
*   **EffectBase (`Source/DSP/EffectBase.h/cpp`):** An abstract base class defining the common interface for all DSP effects. It provides virtual `prepare`, `process`, and `reset` methods, along with common `enabled` and `mix` parameters.
*   **DSP Effects (`Source/DSP/Effects/*.h/cpp`):** Concrete implementations of `EffectBase` for each glitch effect (BitCrusher, BufferStutter, etc.). Each effect implements its specific DSP algorithm and handles parameter updates via a `setParameterValue` method.
*   **UltraGlitchAudioProcessorEditor (`Source/PluginEditor.h/cpp`):** The graphical user interface controller. It inherits from `juce::AudioProcessorEditor`, owns a `MainPanel`, and handles the attachment of GUI controls to the `PluginParameters`.
*   **MainPanel (`Source/GUI/MainPanel.h/cpp`):** The main container for all GUI elements. It arranges `EffectModule`s, a global gain knob, and a chaos button. It receives `PluginParameters` to pass down to its child `EffectModule`s.
*   **EffectModule (`Source/GUI/Components/EffectModule.h/cpp`):** A reusable GUI component representing a single effect. It contains a title, a PowerButton (for enable/disable), and multiple GlitchKnobs for effect-specific parameters. It uses `PluginParameters` to bind its controls.
*   **GlitchKnob (`Source/GUI/Components/GlitchKnob.h/cpp`):** A custom rotary slider with a unique "glitch" aesthetic.
*   **PowerButton (`Source/GUI/Components/PowerButton.h/cpp`):** A custom toggle button, typically used to enable/disable effects, with an LED-style visual feedback.
*   **ChaosButton (`Source/GUI/Components/ChaosButton.h/cpp`):** A custom button specifically for activating/deactivating the CHAOS MODE.
*   **GlitchLookAndFeel (`Source/GUI/LookAndFeel/GlitchLookAndFeel.h/cpp`):** A custom JUCE LookAndFeel class that defines the overall visual style of the plugin, including a dark theme, neon accents, and custom drawing for sliders and buttons.
*   **PluginConfig (`Source/Common/PluginConfig.h`):** Contains global plugin constants like name, version, manufacturer, and audio configuration.
*   **ParameterIDs (`Source/Common/ParameterIDs.h`):** Defines string constants for all plugin parameter IDs, ensuring consistency across the project.
*   **DSPUtils (`Source/Common/DSPUtils.h`):** Provides a collection of inline mathematical and DSP helper functions.

## 3. DSP Algorithms Explained

Each effect implements a specific DSP algorithm:

*   **BitCrusher:**
    *   **Bit Depth Reduction:** Quantizes audio samples by scaling to a smaller integer range, rounding, then scaling back. `quantized_sample = floor(sample * 2^bits) / 2^bits`.
    *   **Sample Rate Decimation:** Holds the output sample constant for a number of input samples, effectively reducing the sample rate. `output_sample[n] = output_sample[n-1]` for `N-1` samples, then `output_sample[n] = input_sample[n]` for the Nth sample.
*   **Buffer Stutter:**
    *   Continuously records incoming audio into a circular buffer.
    *   At a rate determined by `stRate`, a slice of the buffer of `stLength` is "captured".
    *   This captured slice is then repeatedly played back from a temporary buffer until the next trigger.
    *   (Future improvement: Crossfading at slice boundaries to avoid clicks).
*   **Pitch Drift:**
    *   Uses a Low-Frequency Oscillator (LFO, currently sine wave) to modulate a delay line.
    *   The LFO output controls the playback speed of the delay line, creating subtle (or extreme) pitch variations.
    *   `pitch_ratio = 2^(cents / 1200)`.
    *   `modulated_delay_samples = base_delay_samples / pitch_ratio`.
*   **Reverse Slice:**
    *   Accumulates incoming audio into an internal buffer for a duration defined by `rsInterval` (ms).
    *   When the buffer is full, with a probability of `rsChance`, the accumulated audio segment is reversed.
    *   The (possibly reversed) segment is then played back.
*   **Slice Rearrange:**
    *   Takes an entire block of audio (e.g., the current `processBlock` buffer).
    *   Divides it into `srSliceCount` equal segments.
    *   Generates a randomized `sliceOrder` permutation.
    *   Constructs the output by reassembling the segments from the original block according to the `sliceOrder`.
*   **Weird Flanger:**
    *   Combines the input signal with a delayed, feedback-driven copy.
    *   The delay time is continuously modulated by an LFO (`wfRate`, `wfDepth`).
    *   Feedback (`wfFeedback`) from the output of the delay line is fed back into its input, creating resonant comb filter effects.
    *   "Weird" aspects: feedback can be negative (phase inversion), LFO shape (currently sine).
*   **Chaos Controller:**
    *   Does not process audio directly.
    *   When activated, it periodically (rate controlled by `chaosSpeed`) randomizes parameters of other effects.
    *   The `chaosIntensity` parameter controls the likelihood and range of these random changes.
    *   It operates by accessing and setting values via the `PluginParameters` system.

## 4. GUI Structure

The GUI is built around a `MainPanel` which orchestrates several custom components:

*   **`MainPanel`:** Top-level GUI component. Contains a header label, a global output gain `GlitchKnob`, a `ChaosButton`, and a grid of `EffectModule`s.
*   **`EffectModule`:** Each instance represents one of the 7 DSP effects. It displays the effect's name, an `PowerButton` to enable/disable it, and several `GlitchKnob`s for its specific parameters.
*   **`GlitchKnob`:** A custom rotary slider styled by `GlitchLookAndFeel`.
*   **`PowerButton`:** A custom toggle button styled by `GlitchLookAndFeel`, with LED-style feedback for its on/off state.
*   **`ChaosButton`:** A custom `TextButton` styled by `GlitchLookAndFeel`, for engaging/disengaging Chaos Mode.
*   **`GlitchLookAndFeel`:** Applies a consistent "glitch" aesthetic (dark theme, neon colors, custom component drawing) across the entire UI.

## 5. Build System

The project uses CMake for its build system, ensuring cross-platform compatibility and adherence to modern C++ practices.

*   **Framework:** JUCE 8.0.3 (via `FetchContent`).
*   **C++ Standard:** C++17.
*   **CMake Version:** 3.22+.
*   **Targets:** VST3, AU (macOS), Standalone.
*   **macOS Universal Binary:** As of v0.3.0-beta, builds produce fat binaries containing both `arm64` (Apple Silicon) and `x86_64` (Intel/Rosetta) slices. This is enforced in `CMakeLists.txt` via `CMAKE_OSX_ARCHITECTURES "arm64;x86_64"` and requires no command-line flags.

The `CMakeLists.txt` is configured to fetch JUCE automatically, define the plugin properties, enforce universal binary architecture on macOS, and link all source files according to the established directory structure.

### macOS Universal Binary Architecture

macOS Mach-O fat binaries contain multiple architecture slices:
*   **Native Apple Silicon:** macOS loads the `arm64` slice
*   **Rosetta 2:** When a host runs under Rosetta, macOS loads the `x86_64` slice
*   **Intel Mac:** macOS loads the `x86_64` slice natively
*   Both slices share the same bundle ID (`com.ultraglitch.audio.ultraglitch`) and plugin code (`UGBF`), so the DAW sees ONE plugin — no duplicates

## 6. Real-Time Safety Considerations

*   **No Heap Allocations in Audio Thread:** The `processBlock` (and `EffectBase::process`) methods of the processor and effects avoid dynamic memory allocations. All necessary buffers are pre-allocated in `prepareToPlay` (or `EffectBase::prepare`).
*   **No Mutex Locks in Audio Thread:** Thread-safe communication between the GUI (message thread) and the audio processing (real-time audio thread) is achieved primarily through `juce::AudioProcessorValueTreeState` and atomic variables (`std::atomic`) within `EffectBase` and derived classes.
*   **Pre-allocation:** All `juce::AudioBuffer`s and `std::vector`s used for DSP are sized and cleared in `prepareToPlay` or `reset`.
*   **Atomic Access:** Parameters accessed by both GUI and DSP threads (e.g., `EffectBase::enabled`, `dryWetMix`) are `std::atomic` to ensure safe, lock-free access.

## 7. Code Quality

*   **Namespaces:** Classes are organized into `ultraglitch::dsp` for DSP logic and `ultraglitch::gui` for GUI components. Global constants are in `ultraglitch::config` and `ultraglitch::params`.
*   **Naming Conventions:** Adherence to CamelCase for classes and methods, and camelCase for member variables.
*   **RAII:** Resource Acquisition Is Initialization is used where appropriate, leveraging `std::unique_ptr` for resource management.
*   **`const` Correctness:** Extensive use of `const` to indicate immutability and improve code clarity.
*   **`[[nodiscard]]`:** Used on getter methods that return values to encourage clients to use the return value.

---

*End of DOCUMENTATION.md*
