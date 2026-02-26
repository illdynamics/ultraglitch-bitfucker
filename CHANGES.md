# CHANGES.md — UltraGlitch BitFucker

## v0.4.0-beta — Windows Build Hardening + DSP Crash Guards

### Windows Build Hardening
- **COPY_PLUGIN_AFTER_BUILD**: Now platform-conditional — enabled on macOS (user-writable plugin dirs), disabled on Windows and Linux (avoids `Program Files` permission failures in CI and non-admin shells)
- **Separate build directory**: Windows builds now use `build-win/` to avoid colliding with macOS `build/`
- **scripts/build_windows.ps1**: Automated VS2022 x64 configure → build → collect artefacts to `dist/windows-x86_64/`
- **scripts/package_release.sh**: Creates release zip with source + compiled binaries, excludes build intermediates
- **GitHub Actions**: Added `.github/workflows/build.yml` for CI builds on Windows and macOS

### DSP Buffer-Size Crash Guard
- **Root cause**: Multiple effects allocated internal buffers sized to `maxBlockSize` in `prepare()`, then used `copyFrom(... numSamples ...)` in `process()` without guarding for hosts that deliver larger blocks post-prepare
- **Fix pattern**: Added runtime resize guard at the top of `process()` in each affected effect — if `numSamples > buffer.getNumSamples()`, resize once with `setSize(..., numSamples, false, false, true)` (preserving data flag = true for safety)
- **Effects patched**: BufferStutter (dryBuffer_ + stutterOutputBuffer_), PitchDrift (dryBuffer_), ReverseSlice (dryBuffer_), SliceRearrange (dryBuffer_ + blockBuffer_ + processedBuffer_), WeirdFlanger (dryBuffer_)
- **BitCrusher**: Already had the guard since v0.2.2 — no changes needed
- **Impact**: Eliminates the "only crashes when audio is playing" class of errors caused by unexpected block sizes from DAW hosts

### Legacy Cleanup
- **scripts/legacy/**: Moved all Cyqle1-era build scripts (`build.sh`, `configure.sh`, `fetch_juce.sh`, `build_and_test.sh`, `verify_build_environment.cpp`, `analyze_existing_files_and_confirm_build_setup.cpp`, `verify_headers.py`) to `scripts/legacy/` with explanatory README
- **cmake/PluginConfig.cmake**: Renamed to `.legacy` — references Cyqle1 project and is unused by current FetchContent build
- No files deleted — everything preserved for reference

### Version Bump
- All version strings updated to `0.4.0-beta`:
  - `CMakeLists.txt`: `project(UltraGlitch VERSION 0.4.0)`
  - `VERSION`: `v0.4.0-beta`
  - `Source/Common/PluginConfig.h`: `PLUGIN_VERSION_STRING = "0.4.0-beta"`, major=0, minor=4, patch=0
  - `README.md`: Badge updated
  - `BUILD.md`: Header updated with new Windows instructions
  - `CHANGES.md`: This section

## v0.3.0-beta — Universal macOS Binary (arm64 + x86_64)

### Universal Binary Support
- **CMakeLists.txt**: Added `CMAKE_OSX_ARCHITECTURES "arm64;x86_64"` with FORCE cache — enforces universal fat binary build on macOS regardless of host architecture or CLI flags
- **CMakeLists.txt**: `CMAKE_OSX_DEPLOYMENT_TARGET` moved to CACHE with FORCE for consistency
- Single plugin bundle now contains BOTH architecture slices (arm64 + x86_64)
- Produces ONE `.component` (AU), ONE `.vst3` (VST3), and ONE `.app` (Standalone)
- Compatible with: native Apple Silicon Logic Pro, Rosetta-mode Logic Pro, and Intel Macs
- No duplicate plugin entries — same bundle ID, same plugin code, same product name
- Zero DSP code changes required — full source audit confirmed no architecture-specific code

### Architecture Audit (Verified Clean)
- Scanned all 39 source files for: `#ifdef arm64`, `#ifdef x86_64`, `__aarch64__`, `__SSE`, `__AVX`, `__NEON`, `vDSP`, `neon`, `immintrin`, `emmintrin`, SIMD intrinsics
- Result: **ZERO** architecture-specific code found
- All DSP uses portable C++ math (`std::sin`, `std::pow`, `std::floor`, `std::clamp`)
- All JUCE modules (`juce_dsp`, `juce_audio_processors`, `juce_gui_basics`, etc.) are architecture-neutral
- `juce::dsp::DelayLine` (used by PitchDrift) is pure C++ — no platform intrinsics

### Version Bump
- CMake project version: `0.2.7` → `0.3.0`
- VERSION file: `v0.2.7-alpha` → `v0.3.0-beta`
- PluginConfig.h: Updated `PLUGIN_VERSION_STRING`, `PLUGIN_VERSION_MAJOR/MINOR/PATCH`

### Documentation Updates
- BUILD.md: Updated to v0.3.0-beta, added universal binary build instructions, lipo verification steps, and architecture explanation
- README.md: Updated version badge, added universal binary feature, updated build instructions
- DOCUMENTATION.md: Updated build system section with universal binary details

### How It Works (macOS Universal Binary)
- macOS Mach-O fat binaries contain multiple architecture slices in a single file
- When Logic Pro (or any host) loads the plugin natively on Apple Silicon, it selects the `arm64` slice
- When Logic Pro runs under Rosetta 2, it selects the `x86_64` slice
- Since both slices share the same bundle ID (`com.ultraglitch.audio.ultraglitch`) and plugin code (`UGBF`), macOS sees it as ONE plugin — no duplicates
- `lipo -info` on the built binary should report: `Architectures in the fat file: arm64 x86_64`

### Build Verification
```bash
# Build (no arch flags needed — CMakeLists.txt enforces universal)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Verify universal binary
lipo -info build/UltraGlitch_artefacts/Release/AU/UltraGlitch\ BitFucker.component/Contents/MacOS/UltraGlitch\ BitFucker
# Expected: Architectures in the fat file: arm64 x86_64

lipo -info build/UltraGlitch_artefacts/Release/VST3/UltraGlitch\ BitFucker.vst3/Contents/MacOS/UltraGlitch\ BitFucker
# Expected: Architectures in the fat file: arm64 x86_64

# Verify code signing (optional)
codesign -vvv build/UltraGlitch_artefacts/Release/AU/UltraGlitch\ BitFucker.component
```

## v0.2.7-alpha

### Compiler Errors Fixed
- **ChaosButton.cpp**: `isButtonDown()` → `isDown()` — `isButtonDown()` does not exist on `juce::Button`; the correct method is `isDown()`
- **GlitchKnob.cpp**: `juce::Colours::transparent` → `juce::Colours::transparentBlack` — `transparent` is not a member of `juce::Colours`
- **ChaosButton.cpp**: `textColourOnId`/`textColourOffId` — proactively fixed alongside v0.2.6 TextButton colour ID migration

### Proactive Fixes (caught in deep inspection)
- **BufferStutter.cpp**: `triggerIntervalSamples_ = currentSampleRate_ / triggersPerSecond` — `currentSampleRate_` (double) → float precision loss; added `static_cast<float>`

## v0.2.6-alpha

### Compiler Errors Fixed (GlitchLookAndFeel.cpp)
- **Line 33**: `juce::TextButton::textColourId` → `juce::TextButton::textColourOffId` — renamed in JUCE 8
- **drawPopupMenuItem**: `g.fillRoundedRectangle(textRect, 2.0f)` — `textRect` was `Rectangle<int>`, added `.toFloat()` since `fillRoundedRectangle` requires `Rectangle<float>`

### Deprecated API Fixes (GlitchLookAndFeel.cpp)
- All `juce::Font(float)` constructors replaced with `juce::Font(juce::FontOptions(float))` — deprecated in JUCE 8
- Affects: `drawPopupMenuItem`, `drawProgressBar`, `getTextButtonFont`, `getLabelFont`, `getComboBoxFont`

### Unused Parameter Warnings Fixed (GlitchLookAndFeel.cpp)
- `drawPopupMenuItem`: Added `juce::ignoreUnused(shortcutKeyText, icon)`
- `drawComboBox`: Added `buttonY, buttonH` to existing `ignoreUnused` call
- `drawProgressBar`: Added `juce::ignoreUnused(progressBar)`

### Precision Warnings Fixed (PitchDrift.cpp)
- LFO value calculation: `lfoPhase_` (double) now `static_cast<float>` before sin()
- `baseDelaySamples`: `currentSampleRate_` (double) now `static_cast<float>`

### Signedness Warnings Fixed (EffectChain.cpp)
- All `effects_[index]` accesses now use `static_cast<size_t>(index)` to eliminate int→size_type signedness warnings
- `effects_.begin() + index` → `effects_.begin() + static_cast<ptrdiff_t>(index)`

## v0.2.5-alpha

### Compiler Errors Fixed
- **Source/Parameters/PluginParameters.cpp**: Fixed `handle_parameter_change()` — `juce::ListenerList` does not support range-based for loops; replaced `for (auto* listener : parameter_listeners_)` with `parameter_listeners_.call([&](ParameterListener& listener) { ... })` which is the correct JUCE ListenerList iteration API
- **Source/Parameters/PluginParameters.h/.cpp**: Fixed `save_state_to_xml()` — removed `const` qualifier since `APVTS::copyState()` is non-const in JUCE 8.x

## v0.2.4-alpha

### Compiler Errors Fixed
- **Source/PluginProcessor.h**: Added missing `isBusesLayoutSupported()` declaration — the out-of-line definition in .cpp had no matching declaration in the header
- **Source/PluginProcessor.cpp**: Fixed `juce::getXmlFromBinary()` → `getXmlFromBinary()` — this is an inherited `AudioProcessor` member function, not a free function in the `juce::` namespace (JUCE 8.x breaking change)

### Compiler Warnings Fixed
- **Source/DSP/Effects/WeirdFlanger.cpp**: Fixed 3x implicit `double`→`float` precision loss warnings:
  - LFO value calculation: `lfoPhase_` (double) now explicitly cast to float before sin()
  - `minDelaySamples` / `maxDelaySamples`: `currentSampleRate_` (double) now explicitly cast to float

## v0.2.3-alpha

### JuceHeader.h Elimination
- Replaced all 8 occurrences of `#include <JuceHeader.h>` with specific JUCE module includes:
  - `ParameterIDs.h` → `<juce_core/juce_core.h>`
  - `PluginConfig.h` → `<juce_core/juce_core.h>`
  - `GlitchLookAndFeel.h` → `<juce_gui_basics/juce_gui_basics.h>` + `<juce_graphics/juce_graphics.h>`
  - `MainPanel.h` → `<juce_gui_basics/juce_gui_basics.h>`
  - `EffectModule.h` → `<juce_gui_basics/juce_gui_basics.h>` + `<juce_audio_processors/juce_audio_processors.h>`
  - `GlitchKnob.h`, `PowerButton.h`, `ChaosButton.h` → `<juce_gui_basics/juce_gui_basics.h>`
- Removed stale build logs, `.DS_Store` files, `__MACOSX` resource forks, and nested zip artifact

## v0.2.1-alpha Fix Log

## Build System (CMakeLists.txt)
- **FIXED**: Removed conflicting `find_package(JUCE CONFIG REQUIRED)` — now uses FetchContent exclusively
- **FIXED**: Added `juce_dsp` to `target_link_libraries` (needed for PitchDrift's DelayLine)
- **ADDED**: Comprehensive include directories for all source folders
- Version bumped to 0.2.1

## Include Path Corrections
- **Source/Parameters/PluginParameters.cpp**: `../../Common/ParameterIDs.h` → `../Common/ParameterIDs.h`
- **Source/GUI/MainPanel.cpp**: `../../Common/...` → `../Common/...`
- **Source/PluginProcessor.cpp**: Removed all `../Source/` prefixes
- **Source/PluginEditor.h/cpp**: Fixed includes to use correct relative paths
- **Source/GUI/Components/EffectModule.h**: Cleaned up redundant include comments

## Preprocessor / Syntax Errors
- **Source/DSP/Effects/BufferStutter.h**: Fixed `# "../../Common/DSPUtils.h"` → `#include "../../Common/DSPUtils.h"`
- **Source/PluginProcessor.cpp**: Removed broken `#ifndef JucePlugin_PreferredChannelConfigurations` pattern — constructor now always uses BusesProperties directly

## Parameter ID Consistency
- **Source/Common/ParameterIDs.h**: Added `Global_OutputGain = Global_Gain` backward-compatible alias
- **Source/Common/ParameterIDs.h**: Added `SliceRearrange_MAX_SLICE_COUNT = 16` constant
- **Source/Parameters/PluginParameters.cpp**: All parameter creation now uses `juce::ParameterID(id, 1)` for JUCE 7 compatibility
- **Source/Parameters/PluginParameters.cpp**: Fixed `get_parameter_value()` — was incorrectly double-converting via `convertFrom0to1` on already-denormalized `getRawParameterValue`
- **Source/Parameters/PluginParameters.cpp**: Fixed `handle_parameter_change()` — ParameterAttachment callback already provides denormalized value
- **Source/Parameters/PluginParameters.cpp**: Fixed `get_parameter_text()` — now uses `parameter->getValue()` (normalized) for `getText()`
- **Source/Parameters/PluginParameters.cpp**: Fixed `add_listener()`/`remove_listener()` — JUCE's `addParameterListener` requires per-parameter registration

## Namespace / Type Qualifications
- **Source/PluginProcessor.h**: `EffectChain` → `ultraglitch::dsp::EffectChain` (was missing namespace)
- All effect `.h` files: Added `override` keyword to `setParameterValue()` declarations

## LookAndFeel Fixes
- **Source/PluginEditor.h**: Added `GlitchLookAndFeel glitchLookAndFeel_` member (owned instance)
- **Source/PluginEditor.cpp**: Changed to `setDefaultLookAndFeel(&glitchLookAndFeel_)` instead of circular `getDefaultLookAndFeel()` call
- **Source/GUI/MainPanel.cpp**: Removed invalid `setLookAndFeel(&GlitchLookAndFeel::getDefaultLookAndFeel())` call
- **Source/GUI/Components/EffectModule.cpp**: Removed same invalid LookAndFeel call
- **Source/GUI/LookAndFeel/GlitchLookAndFeel.h**: Removed invalid `findColour() const override` (not virtual in JUCE)
- **Source/GUI/LookAndFeel/GlitchLookAndFeel.cpp**: Moved `findColour` overrides into constructor via `setColour()` calls
- **Source/GUI/LookAndFeel/GlitchLookAndFeel.cpp**: Fixed `setDefaultSansSerifTypeface` → `setDefaultSansSerifTypefaceName("Arial")`
- **Source/GUI/LookAndFeel/GlitchLookAndFeel.cpp**: Fixed `fillRect(thumb.withRoundedCorners())` → `fillRoundedRectangle(thumb, 2.0f)` (no such Rectangle method)

## Real-Time Safety Fixes
- **Source/DSP/Effects/ReverseSlice.h/cpp**: Added pre-allocated `processedSliceBuffer_` member; eliminated AudioBuffer allocation in `process()` loop
- **Source/DSP/Effects/SliceRearrange.cpp**: Replaced non-existent `juce::Random::shuffle()` with `std::shuffle(begin, end, randomEngine_)`
- **Source/DSP/Effects/BitCrusher.cpp**: Changed `dryBuffer_.setSize()` in `process()` to use `avoidReallocating=true`
- **Source/DSP/EffectChain.cpp**: Changed `processingBuffer_.setSize()` in `process()` to use `avoidReallocating=true`
- **Source/PluginProcessor.cpp**: Added `juce::ignoreUnused(midiMessages)` and `juce::ScopedNoDenormals`

## GUI Component Fixes
- **Source/GUI/Components/ChaosButton.h/cpp**: Removed `setToggleState(bool)`/`getToggleState()` shadowing that caused APVTS ButtonAttachment desync — now uses base class toggle state correctly
- **Source/GUI/MainPanel.h**: Changed `moduleWidth_`/`moduleHeight_` from `static constexpr` to mutable `int` (modified in `layoutComponents()`)
- **Source/GUI/MainPanel.cpp**: Fixed signed/unsigned comparison warning in `getEffectModule()`
- **Source/GUI/LookAndFeel/GlitchLookAndFeel.cpp**: Added `juce::ignoreUnused()` for unused params in `drawComboBox` and `drawLinearSlider`

## Dead Code Removal
- Removed duplicate root-level `Source/Effects/` and `Source/GUI/GlitchKnob.h` etc. (not referenced by CMake)
- Removed dead `calculateCurrentPitchShift()` declaration from PitchDrift.h

## Files Changed (36 total edits across 25 files)
```
CMakeLists.txt
Source/Common/ParameterIDs.h
Source/DSP/EffectBase.h
Source/DSP/EffectChain.cpp
Source/DSP/Effects/BitCrusher.cpp
Source/DSP/Effects/BitCrusher.h
Source/DSP/Effects/BufferStutter.h
Source/DSP/Effects/ChaosController.h
Source/DSP/Effects/PitchDrift.h
Source/DSP/Effects/ReverseSlice.cpp
Source/DSP/Effects/ReverseSlice.h
Source/DSP/Effects/SliceRearrange.cpp
Source/DSP/Effects/SliceRearrange.h
Source/DSP/Effects/WeirdFlanger.h
Source/GUI/Components/ChaosButton.cpp
Source/GUI/Components/ChaosButton.h
Source/GUI/Components/EffectModule.cpp
Source/GUI/Components/EffectModule.h
Source/GUI/LookAndFeel/GlitchLookAndFeel.cpp
Source/GUI/LookAndFeel/GlitchLookAndFeel.h
Source/GUI/MainPanel.cpp
Source/GUI/MainPanel.h
Source/Parameters/PluginParameters.cpp
Source/PluginEditor.cpp
Source/PluginEditor.h
Source/PluginProcessor.cpp
Source/PluginProcessor.h
```
