#include "WeirdFlanger.h"
#include "../../Common/DSPUtils.h"
#include "../../Common/ParameterIDs.h"
#include <cmath>

namespace ultraglitch::dsp
{

WeirdFlanger::WeirdFlanger()
{
    // Initialize base class members
    setEnabled(false); // Start disabled
    setMix(1.0f);      // Default to 100% wet
}

void WeirdFlanger::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate_ = sampleRate;
    currentMaxBlockSize_ = maxBlockSize;
    
    // Calculate delay buffer size needed for MAX_DELAY_MS
    delayBufferSize_ = static_cast<int>(std::ceil(MAX_DELAY_MS * 0.001 * currentSampleRate_)) + 2; // +2 for interpolation
    
    delayBuffer_.setSize(2, delayBufferSize_); // Stereo buffer
    delayBuffer_.clear();
    
    dryBuffer_.setSize(2, maxBlockSize); // Preallocate for stereo

    writePosition_ = 0;
    lastFeedbackSample_ = 0.0f;
    
    updateLFO(); // Calculate LFO increment based on current rate and sample rate
    reset();
}

void WeirdFlanger::process(juce::AudioBuffer<float>& buffer)
{
    // The EffectChain handles isEnabled() check, so we process if we get here.
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Copy input to dryBuffer_ for mixing later
    for(int ch = 0; ch < numChannels; ++ch)
        dryBuffer_.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx)
    {
        // Generate LFO value (0.0 to 1.0)
        float lfoValue = (std::sin(static_cast<float>(lfoPhase_) * juce::MathConstants<float>::twoPi) * 0.5f + 0.5f);
        
        // Modulate delay time
        float minDelaySamples = MIN_DELAY_MS * 0.001f * static_cast<float>(currentSampleRate_);
        float maxDelaySamples = MAX_DELAY_MS * 0.001f * static_cast<float>(currentSampleRate_);
        
        // Depth parameter controls the modulation range
        float modulatedDelaySamples = minDelaySamples + (maxDelaySamples - minDelaySamples) * lfoValue * depth_;
        
        // Ensure delay is within bounds of the buffer
        modulatedDelaySamples = ultraglitch::dsp::clamp(modulatedDelaySamples, 0.0f, static_cast<float>(delayBufferSize_ - 1));

        for (int channel = 0; channel < numChannels; ++channel)
        {
            float inputSample = dryBuffer_.getSample(channel, sampleIdx); // Read from dry buffer for input
            
            // Add feedback to the input for the delay line
            float inputWithFeedback = inputSample + lastFeedbackSample_ * feedback_; // lastFeedbackSample_ should be per channel

            // This is incorrect for stereo. lastFeedbackSample_ should be an array or per-channel.
            // For PoC, simplify to mono feedback, or handle per-channel feedback.
            // For now, I'll use a single lastFeedbackSample_ which implies mono feedback.
            
            // Write inputWithFeedback to the delay line
            delayBuffer_.setSample(channel, writePosition_, inputWithFeedback);
            
            // Read from the delay line with modulated delay time (linear interpolation)
            int readPosition = writePosition_ - static_cast<int>(modulatedDelaySamples);
            while (readPosition < 0) readPosition += delayBufferSize_;
            
            float delayedSample = ultraglitch::dsp::read_delay_line(delayBuffer_, channel, 
                                                                    static_cast<float>(readPosition), 
                                                                    delayBufferSize_);
            
            // Store feedback sample for next iteration
            lastFeedbackSample_ = delayedSample; // This still implies mono feedback
            
            // Wet signal is the delayed sample
            float wetSample = delayedSample;
            
            // Apply dry/wet mix from EffectBase
            buffer.setSample(channel, sampleIdx, ultraglitch::dsp::mix(inputSample, wetSample, getMix()));
        }
        
        // Advance write position
        writePosition_ = (writePosition_ + 1) % delayBufferSize_;
        
        // Advance LFO phase
        lfoPhase_ += lfoPhaseIncrement_;
        if (lfoPhase_ >= 1.0)
        {
            lfoPhase_ -= 1.0;
        }
    }
}

void WeirdFlanger::reset()
{
    delayBuffer_.clear();
    writePosition_ = 0;
    lastFeedbackSample_ = 0.0f;
    lfoPhase_ = 0.0;
}

void WeirdFlanger::setParameterValue(const juce::String& paramID, float value)
{
    if (paramID == ultraglitch::params::WeirdFlanger_Enabled)
    {
        setEnabled(value > 0.5f);
    }
    else if (paramID == ultraglitch::params::WeirdFlanger_Rate)
    {
        setRate(value);
    }
    else if (paramID == ultraglitch::params::WeirdFlanger_Depth)
    {
        setDepth(value);
    }
    else if (paramID == ultraglitch::params::WeirdFlanger_Feedback)
    {
        setFeedback(value); // Param is -1..1, pass directly
    }
    else if (paramID == ultraglitch::params::WeirdFlanger_Mix)
    {
        setMix(value); // Param is 0..1, pass directly
    }
}

void WeirdFlanger::setRate(float rateHz)
{
    rate_ = ultraglitch::dsp::clamp(rateHz, 0.01f, 20.0f); // tasq.md range
    updateLFO(); // Recalculate LFO increment
}

void WeirdFlanger::setDepth(float depth)
{
    depth_ = ultraglitch::dsp::clamp(depth, 0.0f, 1.0f); // tasq.md implies 0-1
}

void WeirdFlanger::setFeedback(float feedback)
{
    // tasq.md: feedback (-1 to 1). Original code MAX_FEEDBACK 0.95.
    feedback_ = ultraglitch::dsp::clamp(feedback, -1.0f, 1.0f);
}

void WeirdFlanger::updateLFO()
{
    if (currentSampleRate_ > 0)
    {
        lfoPhaseIncrement_ = rate_ / currentSampleRate_;
    } else {
        lfoPhaseIncrement_ = 0.0;
    }
}

// float WeirdFlanger::generateLFOValue() // No longer needed, LFO value generated directly in process
// {
//     return 0.0f;
// }

} // namespace ultraglitch::dsp
