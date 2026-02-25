#pragma once

#include "../EffectBase.h" // Points to ultraglitch::dsp::EffectBase
#include "../../Common/DSPUtils.h" // Points to ultraglitch::dsp::DSPUtils
#include "../../Common/ParameterIDs.h" // For parameter IDs
#include <vector>
#include <random>
#include <algorithm>
#include <numeric> // For std::iota

namespace ultraglitch::dsp
{
class SliceRearrange : public ultraglitch::dsp::EffectBase
{
public:
    SliceRearrange();
    ~SliceRearrange() override = default;

    void prepare(double sampleRate, int maxBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    // Parameter setter from PluginParameters/EffectChain
    void setParameterValue(const juce::String& paramID, float value) override;

    // Specific parameter setters (internal, might be called by setParameterValue)
    void setSliceCount(int count); // srSliceCount (2-16)
    void setRandomizeAmount(float amount); // srRandomize (0-1)

    [[nodiscard]] juce::String getName() const override { return "SliceRearrange"; }

private:
    void updateInternalState(); // Method to recalculate slice sizes and order

    // Parameters
    int sliceCount_ = 4; // srSliceCount (2-16)
    float randomizeAmount_ = 0.5f; // srRandomize (0-1)

    // Internal DSP state
    double currentSampleRate_ = 0.0;
    int currentMaxBlockSize_ = 0;

    // Buffer to hold entire block of audio for slicing
    juce::AudioBuffer<float> blockBuffer_;
    juce::AudioBuffer<float> dryBuffer_;       // Preallocated dry signal buffer
    juce::AudioBuffer<float> processedBuffer_; // Preallocated rearranged output buffer

    // Order of slices to process
    std::vector<int> sliceOrder_; // Stores indices 0 to sliceCount_-1

    // Random number generation
    std::mt19937 randomEngine_; // Standard Mersenne Twister engine
    juce::Random juceRandomGenerator_; // For more convenient JUCE random functions if needed

    // Internal state for block-based processing
    int samplesAccumulated_ = 0; // Samples accumulated for the current blockBuffer_

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliceRearrange)
};
} // namespace ultraglitch::dsp