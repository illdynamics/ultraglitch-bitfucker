# UltraGlitch BitFucker: Cross-Platform Glitch FX Audio Plugin

## Overview

UltraGlitch BitFucker is a multi-effect audio plugin designed for creative glitch production. It provides a suite of destructive and creative effects popular in electronic music, IDM, and sound design, allowing you to mangle and transform your audio in unique ways.

## Features

*   **BitCrusher:** Bit depth reduction and sample rate decimation for digital distortion.
*   **Buffer Stutter:** Rhythmic buffer repeat and stutter effects.
*   **Pitch Drift:** Continuous, subtle, or extreme pitch modulation.
*   **Reverse Slice:** Periodically reverses small segments of audio.
*   **Slice Rearrange:** Chops audio into segments and shuffles their order.
*   **Weird Flanger:** An unconventional flanger with unique feedback characteristics.
*   **CHAOS MODE:** A global mode that continuously randomizes effect parameters for unpredictable sonic mayhem.

## Dependencies

*   **CMake:** Version 3.22 or higher.
*   **C++ Compiler:** C++17 compatible compiler (e.g., GCC, Clang, MSVC).
*   **Git:** For cloning the repository and fetching JUCE.
*   **JUCE Framework:** Version 7.x (automatically fetched via CMake's FetchContent).

## Quick Start

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-repo/ultraglitch-bitfucker.git
    cd ultraglitch-bitfucker
    ```

2.  **Build with CMake:**
    Create a build directory, configure CMake, and build the project.
    ```bash
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release # Or Debug
    cmake --build . --config Release -j$(nproc) # For Linux/macOS
    # For Windows: cmake --build . --config Release -- /m
    ```
    This will compile the plugin for VST3, AU (macOS), and Standalone formats.

3.  **Load in DAW:**
    After a successful build, you will find the plugin files in `build/UltraGlitch_artefacts/`.
    *   **VST3:** Copy `UltraGlitch BitFucker.vst3` to your VST3 plugins folder (e.g., `/Library/Audio/Plug-Ins/VST3/` on macOS, `C:\Program Files\Common Files\VST3\` on Windows).
    *   **AU (macOS):** Copy `UltraGlitch BitFucker.component` to `/Library/Audio/Plug-Ins/Components/`.
    *   **Standalone:** The `UltraGlitch BitFucker` application will be directly available in the artefacts folder.

    Open your Digital Audio Workstation (DAW) and rescan for new plugins if necessary. You should now find "UltraGlitch BitFucker" in your list of effects.

## License

(Add your license information here, e.g., MIT, GPL, etc.)
