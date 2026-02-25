#pragma once

#include "../EffectBase.h" // Points to ultraglitch::dsp::EffectBase
#include "../../Common/DSPUtils.h" // Points to ultraglitch::dsp::DSPUtils
#include "../../Common/ParameterIDs.h" // For parameter IDs
#include <vector> // For circularBuffer_
#include <array>  // For activeSlicesPool_
#include <cmath>

namespace ultraglitch::dsp
{
class BufferStutter : public ultraglitch::dsp::EffectBase {
public:
    BufferStutter();
    ~BufferStutter() override = default;

    void prepare(double sampleRate, int maxBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    // Parameter setter from PluginParameters/EffectChain
    void setParameterValue(const juce::String& paramID, float value) override;

    // Specific parameter setters (internal, might be called by setParameterValue)
    void setStutterRate(float rate); // 1-16 divisions
    void setStutterLength(float lengthMs); // ms

    [[nodiscard]] juce::String getName() const override { return "BufferStutter"; }

private:
    struct StutterSlice {
        int startSample;
        int lengthSamples;
        int currentPosition;
        bool isActive;
        float gain;
        int fadeSamples; // Crossfade length at start/end
    };

    void updateInternalState();
    void triggerNewSlice(); // This needs to be carefully implemented for real-time safety
    void advancePlayback();
    // void applyCrossfade(juce::AudioBuffer<float>& buffer, int startSample, int endSample); // Not used in current impl
    // void fillOutputBuffer(juce::AudioBuffer<float>& buffer); // Not used in current impl

    // Internal state variables
    std::vector<float> circularBuffer_;
    int writePosition_ = 0;
    int bufferSizeSamples_ = 0; // The actual buffer size (capacity)

    static constexpr int MAX_ACTIVE_SLICES = 8; // Max concurrent stutter slices
    std::array<StutterSlice, MAX_ACTIVE_SLICES> activeSlicesPool_;
    int activeSlicesCount_ = 0; // Number of currently active slices

    int sliceTriggerCounter_ = 0;
    int samplesBetweenTriggers_ = 0; // Based on stutter rate (e.g., 1/8th note)

    float stutterRate_ = 1.0f; // in musical divisions (e.g., 8 for 1/8th note)
    float sliceLengthMs_ = 50.0f; // in milliseconds
    
    juce::Random random_; // For randomization if needed

    juce::AudioBuffer<float> dryBuffer_; // Preallocated buffer for dry signal
    // Internal temporary buffer for stuttered audio playback
    juce::AudioBuffer<float> stutterOutputBuffer_;

    // To manage when a slice should trigger
    double currentSampleRate_ = 0.0;
    int samplesPerBlock_ = 0;
    int bufferDurationSamples_ = 0; // How many samples the circular buffer can hold

    // For slice triggering
    float triggerIntervalSamples_ = 0.0f;
    float currentTriggerPhase_ = 0.0f;

    int sliceLengthSamples_ = 0; // Stored here for triggerNewSlice
    // For crossfading (simple linear fade)
    // int crossfadeSamples_ = 0; // Not used in current impl
};
} // namespace ultraglitch::dsp