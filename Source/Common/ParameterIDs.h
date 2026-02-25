#ifndef PARAMETERIDS_H
#define PARAMETERIDS_H

#include <juce_core/juce_core.h>

namespace ultraglitch::params
{
    // Global parameters
    const juce::String Global_Gain = "global_output_gain"; // Changed to match tasq.md
    const juce::String Global_OutputGain = Global_Gain;       // Backward-compatible alias
    const juce::String Global_ChaosMode = "global_chaos_mode"; // From tasq.md: chaosMode bool

    // SliceRearrange constants
    static constexpr int SliceRearrange_MAX_SLICE_COUNT = 16;

    // BitCrusher parameters
    const juce::String BitCrusher_Enabled = "bc_enabled"; // From tasq.md: bcEnabled
    const juce::String BitCrusher_BitDepth = "bc_bit_depth"; // From tasq.md: bcBitDepth
    const juce::String BitCrusher_SampleRateDiv = "bc_samplerate_div"; // From tasq.md: bcSampleRateDiv
    const juce::String BitCrusher_Mix = "bc_mix"; // From tasq.md: bcMix

    // BufferStutter parameters
    const juce::String BufferStutter_Enabled = "st_enabled"; // From tasq.md: stEnabled
    const juce::String BufferStutter_Rate = "st_rate"; // From tasq.md: stRate
    const juce::String BufferStutter_Length = "st_length"; // From tasq.md: stLength
    const juce::String BufferStutter_Mix = "st_mix"; // From tasq.md: stMix

    // PitchDrift parameters
    const juce::String PitchDrift_Enabled = "pd_enabled"; // From tasq.md: pdEnabled
    const juce::String PitchDrift_Amount = "pd_amount"; // From tasq.md: pdAmount
    const juce::String PitchDrift_Speed = "pd_speed"; // From tasq.md: pdSpeed
    const juce::String PitchDrift_Mix = "pd_mix"; // From tasq.md: pdMix

    // ReverseSlice parameters
    const juce::String ReverseSlice_Enabled = "rs_enabled"; // From tasq.md: rsEnabled
    const juce::String ReverseSlice_Interval = "rs_interval"; // From tasq.md: rsInterval
    const juce::String ReverseSlice_Chance = "rs_chance"; // From tasq.md: rsChance
    const juce::String ReverseSlice_Mix = "rs_mix"; // From tasq.md: rsMix

    // SliceRearrange parameters
    const juce::String SliceRearrange_Enabled = "sr_enabled"; // From tasq.md: srEnabled
    const juce::String SliceRearrange_SliceCount = "sr_slice_count"; // From tasq.md: srSliceCount
    const juce::String SliceRearrange_Randomize = "sr_randomize"; // From tasq.md: srRandomize
    const juce::String SliceRearrange_Mix = "sr_mix"; // From tasq.md: srMix

    // WeirdFlanger parameters
    const juce::String WeirdFlanger_Enabled = "wf_enabled"; // From tasq.md: wfEnabled
    const juce::String WeirdFlanger_Rate = "wf_rate"; // From tasq.md: wfRate
    const juce::String WeirdFlanger_Depth = "wf_depth"; // From tasq.md: wfDepth
    const juce::String WeirdFlanger_Feedback = "wf_feedback"; // From tasq.md: wfFeedback
    const juce::String WeirdFlanger_Mix = "wf_mix"; // From tasq.md: wfMix

    // ChaosController parameters (from tasq.md ChaosController section)
    const juce::String ChaosController_Speed = "chaos_speed"; // From tasq.md: chaosSpeed
    const juce::String ChaosController_Intensity = "chaos_intensity"; // From tasq.md: chaosIntensity
}

#endif // PARAMETERIDS_H