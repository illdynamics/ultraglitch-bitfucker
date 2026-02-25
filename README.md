# UltraGlitch BitFucker: Cross-Platform Glitch FX Audio Plugin
[![License: AGPL v3](https://img.shields.io/badge/License-AGPL_v3-blue.svg)](LICENSE)
![Version](https://img.shields.io/badge/version-0.3.0--beta-brightgreen)
![Repo Views](https://komarev.com/ghpvc/?username=illdynamics-ultraglitch-bitfucker&label=Repo+Views&color=blue)

![Splash](ultraglitch-bitfucker-1280x720.jpg)

## Overview

UltraGlitch BitFucker is a multi-effect audio plugin designed for creative glitch production. It provides a suite of destructive and creative effects popular in electronic music, IDM, and sound design, allowing you to mangle and transform your audio in unique ways.

**v0.3.0-beta** builds as a **universal macOS binary** (arm64 + x86_64) — one plugin that works natively on Apple Silicon, under Rosetta 2, and on Intel Macs.

## Features

*   **Universal macOS Binary:** Single plugin supports Apple Silicon native + Intel/Rosetta — no duplicates
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
*   **JUCE Framework:** Version 8.0.3 (automatically fetched via CMake's FetchContent).

## Quick Start

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-repo/ultraglitch-bitfucker.git
    cd ultraglitch-bitfucker
    ```

2.  **Build with CMake:**
    No architecture flags needed — `CMakeLists.txt` enforces universal binary on macOS.
    ```bash
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release
    ```
    This will compile the plugin for VST3, AU (macOS), and Standalone formats as universal binaries (arm64 + x86_64).

3.  **Verify Universal Binary (macOS):**
    ```bash
    lipo -info build/UltraGlitch_artefacts/Release/AU/UltraGlitch\ BitFucker.component/Contents/MacOS/UltraGlitch\ BitFucker
    # Expected: Architectures in the fat file: arm64 x86_64
    ```

4.  **Load in DAW:**
    After a successful build, you will find the plugin files in `build/UltraGlitch_artefacts/`.
    *   **VST3:** Copy `UltraGlitch BitFucker.vst3` to your VST3 plugins folder (e.g., `/Library/Audio/Plug-Ins/VST3/` on macOS, `C:\Program Files\Common Files\VST3\` on Windows).
    *   **AU (macOS):** Copy `UltraGlitch BitFucker.component` to `/Library/Audio/Plug-Ins/Components/`.
    *   **Standalone:** The `UltraGlitch BitFucker` application will be directly available in the artefacts folder.

    Open your Digital Audio Workstation (DAW) and rescan for new plugins if necessary. You should now find "UltraGlitch BitFucker" in your list of effects.

---

## License

UltraGlitch BitFucker is licensed under the GNU Affero General Public License v3.0 (AGPL-3.0).
See the [LICENSE](LICENSE) file for full text.

