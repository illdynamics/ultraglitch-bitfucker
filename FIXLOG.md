# FIXLOG — UltraGlitch BitFucker v0.2.2-alpha

All changes from v0.2.1-alpha → v0.2.2-alpha per MASTERWONQPROMPT v2.

---

## BUG #1 — Effects Do Not Run (Processing Order)

**Root cause:** `rebuildProcessingOrder()` only included *enabled* effects.
All effects start disabled → `processingOrder_` empty at startup → enabling via
APVTS never triggered a rebuild → effects never processed.

**Files changed:**

### Source/DSP/EffectChain.cpp

- `rebuildProcessingOrder()`: Now includes ALL effects with valid pointers
  (regardless of enabled state). Enabled/disabled check moved to `process()` loop.
- `setEffectEnabled()`: Removed unnecessary `rebuildProcessingOrder()` call — order
  is always 0..N-1; the `process()` loop already checks `isEnabled()`.

---

## BUG #2 — Audio-Thread Allocations via get_parameter_definitions()

**Root cause:** `get_parameter_definitions()` returned a fresh `std::vector` by value
on every call. `processBlock()` → `updateEffectChainParameters()` called it every
block, allocating ~30 ParameterDefinition structs per audio callback.

**Files changed:**

### Source/Parameters/PluginParameters.h

- Return type changed: `static std::vector<ParameterDefinition>` →
  `static const std::vector<ParameterDefinition>&`

### Source/Parameters/PluginParameters.cpp

- `get_parameter_definitions()`: Body now uses `static const std::vector<...> definitions = { ... };`
  with `return definitions;` — zero per-call allocation.

### Source/PluginProcessor.cpp

- Constructor: Added `(void) PluginParameters::get_parameter_definitions();` warm-up
  call to ensure static is constructed on main thread before audio starts.

---

## BUG #3 — Parameter Scaling Bugs (Wrong /100 Conversions)

**Root cause:** Several effects divided Mix and Feedback values by 100, but the
parameter definitions already specify 0..1 (Mix) and -1..1 (Feedback) ranges.
Dividing by 100 made controls effectively dead (1% of intended range).

**Files changed:**

### Source/DSP/Effects/BitCrusher.cpp

- `setParameterValue()`: BitCrusher_Mix — removed `/ 100.0f`, pass value directly.

### Source/DSP/Effects/PitchDrift.cpp

- `setParameterValue()`: PitchDrift_Mix — removed `/ 100.0f`, pass value directly.

### Source/DSP/Effects/WeirdFlanger.cpp

- `setParameterValue()`: WeirdFlanger_Feedback — removed `/ 100.0f`, pass directly.
- `setParameterValue()`: WeirdFlanger_Mix — removed `/ 100.0f`, pass directly.

---

## BUG #4 — PitchDrift Amount Range Mismatch

**Root cause:** Parameter definition specifies 0..1200 cents; DSP `setAmount()`
clamped to -100..100 — throwing away 92% of the parameter range.

**Files changed:**

### Source/DSP/Effects/PitchDrift.cpp

- `setAmount()`: Clamp changed from `(-100.0f, 100.0f)` → `(0.0f, 1200.0f)`.

---

## BUG #5 — DBG/Logging + TODO Violations

**Root cause:** `ChaosController.cpp` had `DBG()` calls (logging on audio thread)
and unimplemented TODO comments.

**Files changed:**

### Source/DSP/Effects/ChaosController.cpp

- `setParameterValue(Global_ChaosMode)`: Removed DBG. Implemented clean chaos-off:
  tracks `wasEnabled`, resets `samplesCounter_` and clears
  `shouldTriggerRandomization_` flag on disable.
- `randomizeParameters()`: Removed DBG for null `pluginParameters_`.
- `setParameterValue(ChaosController_Intensity)`: Fixed conversion — param is 0..100%,
  DSP expects 0..1 → added `/ 100.0f` (this one is correct).
- All TODO comments removed.

---

## BUG #6 — EffectChain Placeholders

**Files changed:**

### Source/DSP/EffectChain.cpp

- `getEffectName()`: Implemented using `effect->getName()` (was returning empty).
- `saveState()`: Removed TODO comments, added effect Name attribute, documented
  that per-effect parameter state is via APVTS.
- `loadState()`: Removed TODO comments and "Note:" placeholder.

---

## BUILD SYSTEM HARDENING

**Files changed:**

### CMakeLists.txt

- Version bumped: `0.2.1` → `0.2.2`
- Plugin formats now platform-conditional:
  - `APPLE`: VST3 + AU + Standalone
  - Otherwise: VST3 + Standalone
- Uses `${ULTRAGLITCH_FORMATS}` variable in `juce_add_plugin()`.

---

## DSP SANITY — Anti-Click Crossfades

**Files changed:**

### Source/Common/DSPUtils.h

- Added `CROSSFADE_SAMPLES = 32` constant (~0.7ms at 44.1kHz).
- Added `apply_fade_in()` — linear ramp with zero-guard on fadeLengthSamples.
- Added `apply_fade_out()` — linear ramp with zero-guard on fadeLengthSamples.
- Added `apply_slice_crossfade()` — convenience wrapper applying both fades to a
  buffer region, with minimum-length check.

### Source/DSP/Effects/ReverseSlice.cpp

- `process()`: Applied `apply_slice_crossfade()` after `reverseAudioSegment()`.

### Source/DSP/Effects/SliceRearrange.cpp

- `process()`: Applied `apply_slice_crossfade()` after each slice copy to
  `processedBuffer_`.

### Source/DSP/Effects/BufferStutter.h

- `StutterSlice` struct: Added `fadeSamples` member.

### Source/DSP/Effects/BufferStutter.cpp

- `prepare()`: Initializes `fadeSamples = 0` in pool.
- `triggerNewSlice()`: Sets `fadeSamples = min(CROSSFADE_SAMPLES, sliceLen/4)`.
- `process()` slice loop: Applies per-sample fade envelope using `fadeSamples`,
  with `fadeSamples > 0` guard to prevent division by zero.

---

## SUMMARY

| Category | Files Changed | Edits |
|----------|--------------|-------|
| Bug #1 — Processing order | 1 | 2 |
| Bug #2 — RT allocations | 3 | 3 |
| Bug #3 — Param scaling | 3 | 4 |
| Bug #4 — PitchDrift range | 1 | 1 |
| Bug #5 — DBG/TODO | 1 | 3 |
| Bug #6 — Placeholders | 1 | 3 |
| Build system | 1 | 2 |
| DSP crossfades | 5 | 8 |
| **Total** | **12 unique files** | **26 edits** |
