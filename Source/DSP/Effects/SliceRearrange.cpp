#include "SliceRearrange.h"
#include "../../Common/ParameterIDs.h"
#include "../../Common/DSPUtils.h" // For ultraglitch::dsp::clamp and mix
#include <numeric> // For std::iota

namespace ultraglitch::dsp
{

SliceRearrange::SliceRearrange()
    : randomEngine_(std::random_device{}()), // Initialize std::mt19937
      juceRandomGenerator_(juce::Time::currentTimeMillis()) // Initialize juce::Random
{
    // Initialize base class members
    setEnabled(false); // Start disabled
    setMix(0.0f);      // Default to 0% wet as per tasq.md
}

void SliceRearrange::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate_ = sampleRate;
    currentMaxBlockSize_ = maxBlockSize;
    
    // Preallocate internal buffers for real-time safety
    dryBuffer_.setSize(2, maxBlockSize);
    dryBuffer_.clear();
    blockBuffer_.setSize(2, maxBlockSize); // Buffer to hold the entire input block for processing
    blockBuffer_.clear();
    processedBuffer_.setSize(2, maxBlockSize); // Buffer to build the rearranged output
    processedBuffer_.clear();

    // Ensure sliceOrder_ has enough capacity
    sliceOrder_.reserve(ultraglitch::params::SliceRearrange_MAX_SLICE_COUNT);

    updateInternalState(); // Calculate slice boundaries and initial order
    reset();
}

void SliceRearrange::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Copy input to dryBuffer_ for mixing later
    for(int ch = 0; ch < numChannels; ++ch)
        dryBuffer_.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    
    // Copy input buffer to our internal blockBuffer_
    for (int channel = 0; channel < numChannels; ++channel)
    {
        blockBuffer_.copyFrom(channel, 0, buffer, channel, 0, numSamples);
    }

    // Generate new slice order if randomizeAmount_ is active
    // This is done once per process block for simplicity in PoC
    if (randomizeAmount_ > 0.0f) // Only randomize if randomizeAmount is active
    {
        updateInternalState(); // This will regenerate slice order
    }
    
    processedBuffer_.clear(); // Clear the buffer where we'll build the rearranged output

    // Calculate slice parameters
    int samplesPerSlice = numSamples / sliceCount_;
    int remainder = numSamples % sliceCount_;

    // Iterate through the randomized slice order and copy slices from blockBuffer_ to processedBuffer_
    int currentOutputPos = 0;
    for (int orderedSliceIndex : sliceOrder_)
    {
        int actualSliceLength = samplesPerSlice;
        if (orderedSliceIndex < remainder) // Distribute remainder samples among first slices
            actualSliceLength++;
        
        // Calculate start position of the current slice in the original blockBuffer_
        int currentSliceStartInBlock = 0;
        for (int i = 0; i < orderedSliceIndex; ++i)
        {
            int prevSliceLength = samplesPerSlice;
            if (i < remainder)
                prevSliceLength++;
            currentSliceStartInBlock += prevSliceLength;
        }

        // Copy slice from blockBuffer_ to processedBuffer_
        if (actualSliceLength > 0 && currentSliceStartInBlock + actualSliceLength <= numSamples)
        {
            for (int channel = 0; channel < numChannels; ++channel)
            {
                processedBuffer_.copyFrom(channel, currentOutputPos, blockBuffer_, channel, currentSliceStartInBlock, actualSliceLength);
            }
            // Apply crossfade at slice boundaries to smooth transitions
            ultraglitch::dsp::apply_slice_crossfade(processedBuffer_, currentOutputPos, actualSliceLength);
            currentOutputPos += actualSliceLength;
        }
    }

    // Apply dry/wet mix from EffectBase
    float currentMix = getMix();
    if (currentMix == 0.0f) return; // Completely dry, no need to mix

    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* outputChannelData = buffer.getWritePointer(channel);
        const float* dryChannelData = dryBuffer_.getReadPointer(channel);
        const float* wetChannelData = processedBuffer_.getReadPointer(channel);

        for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx)
        {
            outputChannelData[sampleIdx] = ultraglitch::dsp::mix(dryChannelData[sampleIdx], wetChannelData[sampleIdx], currentMix);
        }
    }
}

void SliceRearrange::reset()
{
    blockBuffer_.clear();
    processedBuffer_.clear();
    samplesAccumulated_ = 0;
    // Reset random engine if desired, or let it continue its sequence
    // randomEngine_.seed(juce::Time::currentTimeMillis()); // Re-seed if needed
}

void SliceRearrange::setParameterValue(const juce::String& paramID, float value)
{
    if (paramID == ultraglitch::params::SliceRearrange_Enabled)
    {
        setEnabled(value > 0.5f);
    }
    else if (paramID == ultraglitch::params::SliceRearrange_SliceCount)
    {
        setSliceCount(static_cast<int>(value));
    }
    else if (paramID == ultraglitch::params::SliceRearrange_Randomize)
    {
        setRandomizeAmount(value); // Value is already 0-1
    }
    else if (paramID == ultraglitch::params::SliceRearrange_Mix)
    {
        setMix(value); // Mix parameter is 0-1
    }
}

void SliceRearrange::setSliceCount(int count)
{
    sliceCount_ = ultraglitch::dsp::clamp(count, 2, 16); // tasq.md range
    updateInternalState(); // Recalculate slice order
}

void SliceRearrange::setRandomizeAmount(float amount)
{
    randomizeAmount_ = ultraglitch::dsp::clamp(amount, 0.0f, 1.0f);
    // updateInternalState() will be called in process if randomizeAmount_ > 0
    // or when sliceCount_ changes.
}

void SliceRearrange::updateInternalState()
{
    // Generate slice order based on sliceCount_ and randomizeAmount_
    sliceOrder_.clear();
    for (int i = 0; i < sliceCount_; ++i)
    {
        sliceOrder_.push_back(i);
    }

    // Shuffle based on randomizeAmount_
    // If randomizeAmount_ is 0, then no shuffle occurs.
    // If randomizeAmount_ is > 0, we shuffle. For more nuanced control,
    // this would be a probability check or a specific mapping.
    if (randomizeAmount_ > 0.0f)
    {
        // Shuffle using std::shuffle with our Mersenne Twister engine (RT-safe, no allocation)
        std::shuffle(sliceOrder_.begin(), sliceOrder_.end(), randomEngine_);
    }
}

} // namespace ultraglitch::dsp