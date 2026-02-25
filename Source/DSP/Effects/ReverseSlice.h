#pragma once

#include "../EffectBase.h"
#include "../../Common/DSPUtils.h"
#include "../../Common/ParameterIDs.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

namespace ultraglitch::dsp
{

class ReverseSlice : public ultraglitch::dsp::EffectBase
{
public:
    ReverseSlice();
    ~ReverseSlice() override = default;

    void prepare(double sampleRate, int maxBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    void setParameterValue(const juce::String& paramID, float value) override;

    void setSliceIntervalMs(float intervalMs);
    void setReverseChance(float chance);

    [[nodiscard]] juce::String getName() const override { return "ReverseSlice"; }

private:
    void updateInternalState();
    void reverseAudioSegment(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    juce::AudioBuffer<float> internalBuffer_;
    juce::AudioBuffer<float> processedSliceBuffer_;
    juce::AudioBuffer<float> pendingSliceBuffer_;
    juce::AudioBuffer<float> dryBuffer_;

    juce::Random randomGenerator_;

    double currentSampleRate_ = 0.0;
    int currentMaxBlockSize_ = 0;

    int sliceIntervalSamples_ = 0;
    float sliceIntervalMs_ = 100.0f;
    float reverseChance_ = 0.5f;

    int samplesSinceLastSlice_ = 0;

    bool isPlayingSlice_ = false;
    bool pendingSliceReady_ = false;
    int playheadInSlice_ = 0;

    static constexpr int MAX_SLICE_BUFFER_SAMPLES = 48000;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverseSlice)
};

}
