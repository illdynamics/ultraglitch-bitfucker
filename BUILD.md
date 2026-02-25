# BUILD — UltraGlitch BitFucker v0.2.2-alpha

## Prerequisites

- **CMake** 3.22+
- **C++17** compatible compiler
- **Git** (JUCE is fetched via FetchContent)
- No pre-installed JUCE required

## macOS

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
