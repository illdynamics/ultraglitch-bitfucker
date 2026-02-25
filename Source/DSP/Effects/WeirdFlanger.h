#pragma once

#include "../EffectBase.h" // Points to ultraglitch::dsp::EffectBase
#include "../../Common/DSPUtils.h" // Points to ultraglitch::dsp::DSPUtils
#include "../../Common/ParameterIDs.h" // For parameter IDs
#include <juce_dsp/juce_dsp.h> // For juce::dsp::DelayLine or other dsp utilities
#include <juce_audio_basics/juce_audio_basics.h> // For juce::AudioBuffer

namespace ultraglitch::dsp
{
class WeirdFlanger : public ultraglitch::dsp::EffectBase
{
public:
    WeirdFlanger();
    ~WeirdFlanger() override = default;

    void prepare(double sampleRate, int maxBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    // Parameter setter from PluginParameters/EffectChain
    void setParameterValue(const juce::String& paramID, float value) override;

    // Specific parameter setters (internal, might be called by setParameterValue)
    void setRate(float rateHz); // wfRate (Hz)
    void setDepth(float depth); // wfDepth
    void setFeedback(float feedback); // wfFeedback (-1 to 1)

    [[nodiscard]] juce::String getName() const override { return "WeirdFlanger"; }

private:
    void updateLFO(); // Method to update LFO rate and phase increment

    // Parameters
    float rate_ = 1.0f; // LFO rate in Hz
    float depth_ = 0.5f; // LFO depth (modulates delay time range)
    float feedback_ = 0.0f; // Feedback amount (-1.0 to 1.0)

    double currentSampleRate_ = 0.0;
    int currentMaxBlockSize_ = 0;

    // LFO state
    double lfoPhase_ = 0.0;
    double lfoPhaseIncrement_ = 0.0;

    // Delay line for flanger effect
    juce::AudioBuffer<float> delayBuffer_; // Manual circular buffer
    int delayBufferSize_ = 0;
    int writePosition_ = 0;

    float lastFeedbackSample_ = 0.0f; // Stores the last sample from the delay line for feedback

    juce::AudioBuffer<float> dryBuffer_; // Preallocated buffer for dry signal

    // Flanger specific constants
    static constexpr float MIN_DELAY_MS = 0.5f; // Min delay in milliseconds
    static constexpr float MAX_DELAY_MS = 10.0f; // Max delay in milliseconds

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WeirdFlanger)
};
} // namespace ultraglitch::dsp