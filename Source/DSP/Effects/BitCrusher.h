#pragma once

#include "../EffectBase.h" // Points to ultraglitch::dsp::EffectBase
#include "../../Common/DSPUtils.h" // Points to ultraglitch::dsp::DSPUtils
#include "../../Common/ParameterIDs.h" // For parameter IDs

namespace ultraglitch::dsp
{
class BitCrusher : public ultraglitch::dsp::EffectBase
{
public:
    BitCrusher();
    ~BitCrusher() override = default;

    void prepare(double sampleRate, int maxBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    // Parameter setters from PluginParameters/EffectChain
    void setParameterValue(const juce::String& paramID, float value) override;

    // Specific parameter setters (internal, might be called by setParameterValue)
    void setBitDepth(float bits);
    void setSampleRateReduction(float reductionFactor);

    [[nodiscard]] juce::String getName() const override { return "BitCrusher"; } // Added for EffectChain

private:
    float bitDepth_ = 16.0f;
    float sampleRateReductionFactor_ = 1.0f;

    double currentSampleRate_ = 0.0;
    int currentMaxBlockSize_ = 0;

    // Internal DSP state
    int reductionCounter_ = 0;
    float holdValue_ = 0.0f;
    juce::AudioBuffer<float> dryBuffer_; // Preallocated buffer for dry signal

    float applyBitCrushing(float sample);
    void updateInternalState(); // Method to recalculate internal DSP values based on parameters
    
    // JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitCrusher) // Not needed for derived class unless specific non-copyable requirements
};
} // namespace ultraglitch::dsp