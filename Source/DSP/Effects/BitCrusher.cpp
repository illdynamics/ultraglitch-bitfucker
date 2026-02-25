#include "BitCrusher.h"
#include "../../Common/ParameterIDs.h"

namespace ultraglitch::dsp
{

BitCrusher::BitCrusher()
    : bitDepth_(16.0f),
      sampleRateReductionFactor_(1.0f),
      currentSampleRate_(0.0),
      currentMaxBlockSize_(0),
      reductionCounter_(0),
      holdValue_(0.0f)
{
    // Initialize base class members
    setEnabled(false); // Start disabled
    setMix(1.0f);      // Default to 100% wet
}

void BitCrusher::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate_ = sampleRate;
    currentMaxBlockSize_ = maxBlockSize;
    dryBuffer_.setSize(2, maxBlockSize); // Preallocate for stereo
    reset();
}

void BitCrusher::process(juce::AudioBuffer<float>& buffer)
{
    // The EffectChain handles isEnabled() check, so we process if we get here.
    // However, the individual effects' Mix parameter will be handled here.
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Resize dryBuffer_ if channel count changes (should not happen in prepareToPlay)
    // or if maxBlockSize changes (should not happen in processBlock)
    if (dryBuffer_.getNumChannels() < numChannels || dryBuffer_.getNumSamples() < numSamples)
    {
        // Use avoidReallocating=true to prevent RT allocation when possible
        dryBuffer_.setSize(numChannels, numSamples, false, false, true);
    }

    // Copy input to preallocated dryBuffer_ for mixing
    for (int channel = 0; channel < numChannels; ++channel)
    {
        dryBuffer_.copyFrom(channel, 0, buffer, channel, 0, numSamples);
    }

    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx)
        {
            float inputSample = channelData[sampleIdx];
            float processedSample = inputSample; // Start with input_sample for processing

            if (reductionCounter_ == 0)
            {
                holdValue_ = applyBitCrushing(inputSample);
            }
            
            processedSample = holdValue_;
            
            // Increment reduction counter and reset if necessary for sample rate reduction
            reductionCounter_++;
            if (reductionCounter_ >= static_cast<int>(sampleRateReductionFactor_))
            {
                reductionCounter_ = 0;
            }

            // Apply dry/wet mix from EffectBase
            channelData[sampleIdx] = ultraglitch::dsp::mix(dryBuffer_.getSample(channel, sampleIdx), processedSample, getMix());
        }
    }
}

void BitCrusher::reset()
{
    reductionCounter_ = 0;
    holdValue_ = 0.0f;
}

void BitCrusher::setParameterValue(const juce::String& paramID, float value)
{
    if (paramID == ultraglitch::params::BitCrusher_Enabled)
    {
        setEnabled(value > 0.5f);
    }
    else if (paramID == ultraglitch::params::BitCrusher_BitDepth)
    {
        setBitDepth(value);
    }
    else if (paramID == ultraglitch::params::BitCrusher_SampleRateDiv)
    {
        setSampleRateReduction(value);
    }
    else if (paramID == ultraglitch::params::BitCrusher_Mix)
    {
        setMix(value); // Param is 0..1, pass directly
    }
}

void BitCrusher::setBitDepth(float bits)
{
    bitDepth_ = ultraglitch::dsp::clamp(bits, 1.0f, 16.0f);
    updateInternalState();
}

void BitCrusher::setSampleRateReduction(float reductionFactor)
{
    sampleRateReductionFactor_ = ultraglitch::dsp::clamp(reductionFactor, 1.0f, 64.0f); // Max 64 as per tasq.md
    updateInternalState();
}

float BitCrusher::applyBitCrushing(float sample)
{
    if (bitDepth_ >= 16.0f) // No bit crushing if depth is max or higher
    {
        return sample;
    }
    
    // Scale factor to effectively quantize the sample
    const float maxQuantizedValue = std::pow(2.0f, bitDepth_); // Number of discrete levels
    const float quantizedSample = std::floor(sample * maxQuantizedValue) / maxQuantizedValue;
    
    return quantizedSample;
}

void BitCrusher::updateInternalState()
{
    // This method can be used to recalculate any internal state variables that depend on parameters
    // For bitcrusher, just resetting the counter is sufficient for some changes.
    reductionCounter_ = 0;
}

} // namespace ultraglitch::dsp