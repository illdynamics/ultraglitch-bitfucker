# BUILD — UltraGlitch BitFucker v0.3.0-beta

## Prerequisites

- **CMake** 3.22+
- **C++17** compatible compiler
- **Git** (JUCE is fetched via FetchContent)
- No pre-installed JUCE required

## macOS (Universal Binary — arm64 + x86_64)

As of v0.3.0-beta, the build system automatically produces **universal binaries** containing both Apple Silicon (arm64) and Intel (x86_64) architecture slices. No architecture flags are needed on the command line — `CMakeLists.txt` enforces this via `CMAKE_OSX_ARCHITECTURES`.

### Xcode Generator (recommended)

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

### Makefile Generator

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(sysctl -n hw.ncpu)
```

### Verify Universal Binary

After building, confirm both architecture slices are present:

```bash
lipo -info build/UltraGlitch_artefacts/Release/AU/UltraGlitch\ BitFucker.component/Contents/MacOS/UltraGlitch\ BitFucker
# Expected: Architectures in the fat file: arm64 x86_64

lipo -info build/UltraGlitch_artefacts/Release/VST3/UltraGlitch\ BitFucker.vst3/Contents/MacOS/UltraGlitch\ BitFucker
# Expected: Architectures in the fat file: arm64 x86_64
```

### Output Artefacts (macOS)

```
build/UltraGlitch_artefacts/Release/VST3/UltraGlitch BitFucker.vst3
build/UltraGlitch_artefacts/Release/AU/UltraGlitch BitFucker.component
build/UltraGlitch_artefacts/Release/Standalone/UltraGlitch BitFucker.app
```

With `COPY_PLUGIN_AFTER_BUILD TRUE`, artefacts are also copied to:
```
~/Library/Audio/Plug-Ins/VST3/
~/Library/Audio/Plug-Ins/Components/
```

### How Universal Binaries Work

macOS fat (Mach-O) binaries contain multiple architecture slices in a single file:

- **Native Apple Silicon (M1/M2/M3):** macOS loads the `arm64` slice automatically
- **Rosetta 2 mode:** When Logic Pro (or any host) runs under Rosetta, macOS loads the `x86_64` slice
- **Intel Mac:** macOS loads the `x86_64` slice natively
- **One bundle ID, one plugin:** Both slices share the same `com.ultraglitch.audio.ultraglitch` identifier — no duplicate entries in the DAW plugin list

### Clean Build (if needed)

```bash
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Windows

### Visual Studio 2022

```powershell
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

### Output Artefacts (Windows)

```
build\UltraGlitch_artefacts\Release\VST3\UltraGlitch BitFucker.vst3
build\UltraGlitch_artefacts\Release\Standalone\UltraGlitch BitFucker.exe
```

Note: AU is not built on Windows (macOS only).

## Linux

```bash
# Install dependencies first:
sudo apt install -y libasound2-dev libcurl4-openssl-dev libfreetype6-dev \
    libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev \
    libxrandr-dev libxrender-dev libwebkit2gtk-4.0-dev libglu1-mesa-dev

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Output Artefacts (Linux)

```
build/UltraGlitch_artefacts/Release/VST3/UltraGlitch BitFucker.vst3
build/UltraGlitch_artefacts/Release/Standalone/UltraGlitch BitFucker
```

## Troubleshooting

| Issue | Fix |
|-------|-----|
| JUCE FetchContent timeout | Run `cmake -B build` again (network retry) |
| Xcode: "No signing identity" | Build > Signing & Capabilities > Disable signing |
| Linux: missing headers | Install the full dependency list above |
| CMake < 3.22 | Upgrade: `pip install cmake --upgrade` or `brew install cmake` |
