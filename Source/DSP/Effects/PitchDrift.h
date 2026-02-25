#pragma once

#include "../EffectBase.h" // Points to ultraglitch::dsp::EffectBase
#include "../../Common/DSPUtils.h" // Points to ultraglitch::dsp::DSPUtils
#include "../../Common/ParameterIDs.h" // For parameter IDs
#include <juce_dsp/juce_dsp.h> // For juce::dsp::DelayLine

namespace ultraglitch::dsp
{
class PitchDrift : public ultraglitch::dsp::EffectBase
{
public:
    PitchDrift();
    ~PitchDrift() override = default;

    void prepare(double sampleRate, int maxBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    // Parameter setter from PluginParameters/EffectChain
    void setParameterValue(const juce::String& paramID, float value) override;

    // Specific parameter setters (internal, might be called by setParameterValue)
    void setAmount(float amountCents); // pdAmount (cents)
    void setSpeed(float speedHz); // pdSpeed (Hz)

    [[nodiscard]] juce::String getName() const override { return "PitchDrift"; }

private:
    void updateLFO(); // Method to update LFO frequency and phase

    double currentSampleRate_ = 0.0;
    int currentMaxBlockSize_ = 0;

    // Parameters
    float amountCents_ = 0.0f; // Total pitch deviation in cents (e.g., +/- 100 cents)
    float speedHz_ = 1.0f; // LFO speed in Hz

    // LFO state
    double lfoPhase_ = 0.0;
    double lfoPhaseIncrement_ = 0.0; // In radians per sample

    // Delay line for pitch shifting
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine_;
    static constexpr int MAX_DELAY_SAMPLES = 88200; // Max 2 seconds delay at 44.1kHz * 2 for +/- pitch shift

    juce::Random randomGenerator_; // For potential random LFO or other variations
    juce::AudioBuffer<float> dryBuffer_; // Preallocated buffer for dry signal

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchDrift)
};
} // namespace ultraglitch::dsp