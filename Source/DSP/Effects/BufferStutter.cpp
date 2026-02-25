#include "BufferStutter.h"
#include "../../Common/ParameterIDs.h"
#include "../../Common/DSPUtils.h" // For ultraglitch::dsp::clamp and mix
#include <limits> // For std::numeric_limits

namespace ultraglitch::dsp
{

BufferStutter::BufferStutter()
    : random_() // Initialize juce::Random
{
    // Initialize base class members
    setEnabled(false); // Start disabled
    setMix(0.0f);      // Default to 0% wet as per tasq.md
}

void BufferStutter::prepare(double sampleRate, int maxBlockSize)
{
    // ---- Sanity Guards ----
    if (sampleRate <= 0.0 || sampleRate > 384000.0)
        return;

    if (maxBlockSize <= 0 || maxBlockSize > 8192)
        return;

    currentSampleRate_ = sampleRate;
    samplesPerBlock_   = maxBlockSize;

    // ---- Safe circular buffer allocation ----
    constexpr double maxBufferSeconds = 10.0;   // hard cap
    constexpr double defaultSeconds   = 2.0;    // intended length

    const double clampedSeconds =
        juce::jlimit(0.1, maxBufferSeconds, defaultSeconds);

    const int safeSize = static_cast<int>(currentSampleRate_ * clampedSeconds);

    if (safeSize <= 0 || safeSize > static_cast<int>(currentSampleRate_ * maxBufferSeconds))
        return;

    bufferDurationSamples_ = safeSize;

    // Use assign instead of resize to force clean reallocation
    circularBuffer_.assign(bufferDurationSamples_, 0.0f);

    // ---- Preallocate working buffers safely ----
    dryBuffer_.setSize(2, maxBlockSize, false, false, true);
    dryBuffer_.clear();

    stutterOutputBuffer_.setSize(2, maxBlockSize, false, false, true);
    stutterOutputBuffer_.clear();

    // ---- Reset slice pool safely ----
    for (int i = 0; i < MAX_ACTIVE_SLICES; ++i)
    {
        activeSlicesPool_[i].isActive    = false;
        activeSlicesPool_[i].fadeSamples = 0;
    }

    activeSlicesCount_ = 0;

    // ---- Reset internal state to prevent stale pointer behaviour ----
    writePosition_ = 0;
// ----    readPosition_  = 0; ----
}

void BufferStutter::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Copy input to dryBuffer_ for mixing later
    for(int ch = 0; ch < numChannels; ++ch)
        dryBuffer_.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    
    stutterOutputBuffer_.clear(); // Clear wet buffer for current block

    for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx)
    {
        // 1. Record incoming audio to circular buffer (mono sum for simplicity)
        float monoInput = 0.0f;
        for(int ch = 0; ch < numChannels; ++ch)
            monoInput += buffer.getSample(ch, sampleIdx);
        monoInput /= static_cast<float>(numChannels);

        circularBuffer_[writePosition_] = monoInput;
        writePosition_ = (writePosition_ + 1) % bufferDurationSamples_;

        // 2. Trigger new slice based on rate
        currentTriggerPhase_ += 1.0f; // Advance by one sample
        if (currentTriggerPhase_ >= triggerIntervalSamples_)
        {
            triggerNewSlice();
            currentTriggerPhase_ -= triggerIntervalSamples_; // Subtract to maintain phase
        }

        // 3. Advance active slices and sum their output to stutterOutputBuffer_
        for (int i = 0; i < activeSlicesCount_; ++i) // Iterate only active slices
        {
            StutterSlice& slice = activeSlicesPool_[i];
            if (slice.isActive)
            {
                int readPos = (slice.startSample + slice.currentPosition) % bufferDurationSamples_;
                float sampleValue = circularBuffer_[readPos];
                
                // Apply crossfade envelope at slice boundaries
                float fadeGain = 1.0f;
                if (slice.fadeSamples > 0)
                {
                    if (slice.currentPosition < slice.fadeSamples)
                        fadeGain = static_cast<float>(slice.currentPosition) / static_cast<float>(slice.fadeSamples);
                    else if (slice.currentPosition >= slice.lengthSamples - slice.fadeSamples)
                        fadeGain = static_cast<float>(slice.lengthSamples - slice.currentPosition) / static_cast<float>(slice.fadeSamples);
                }
                
                // Add sample to the wet buffer (stereo)
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    stutterOutputBuffer_.addSample(ch, sampleIdx, sampleValue * slice.gain * fadeGain);
                }

                slice.currentPosition++;

                if (slice.currentPosition >= slice.lengthSamples)
                {
                    slice.isActive = false; // Deactivate when done
                }
            }
        }
        
        // Remove inactive slices from the active set by compacting the active_slices_pool_
        // This process can be made more efficient with a different data structure,
        // but for a PoC this is safe as activeSlicesCount_ is small.
        int writeIdx = 0;
        for (int readIdx = 0; readIdx < activeSlicesCount_; ++readIdx)
        {
            if (activeSlicesPool_[readIdx].isActive)
            {
                if (writeIdx != readIdx)
                {
                    activeSlicesPool_[writeIdx] = activeSlicesPool_[readIdx];
                }
                writeIdx++;
            }
        }
        activeSlicesCount_ = writeIdx;

    } // End of sample loop

    // 4. Mix original dryBuffer_ with processed stutterOutputBuffer_
    float currentMix = getMix(); // Get mix from EffectBase
    if (currentMix == 0.0f) return; // Completely dry, no need to mix

    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* outputChannelData = buffer.getWritePointer(channel);
        const float* dryChannelData = dryBuffer_.getReadPointer(channel);
        const float* stutterWetData = stutterOutputBuffer_.getReadPointer(channel);

        for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx)
        {
            outputChannelData[sampleIdx] = ultraglitch::dsp::mix(dryChannelData[sampleIdx], stutterWetData[sampleIdx], currentMix);
        }
    }
}

void BufferStutter::reset()
{
    std::fill(circularBuffer_.begin(), circularBuffer_.end(), 0.0f);
    writePosition_ = 0;
    
    // Reset active slices pool
    for (int i = 0; i < MAX_ACTIVE_SLICES; ++i)
    {
        activeSlicesPool_[i].isActive = false;
    }
    activeSlicesCount_ = 0;

    currentTriggerPhase_ = 0.0f;
    stutterOutputBuffer_.clear();
}

void BufferStutter::setParameterValue(const juce::String& paramID, float value)
{
    if (paramID == ultraglitch::params::BufferStutter_Enabled)
    {
        setEnabled(value > 0.5f);
    }
    else if (paramID == ultraglitch::params::BufferStutter_Rate)
    {
        setStutterRate(value);
    }
    else if (paramID == ultraglitch::params::BufferStutter_Length)
    {
        setStutterLength(value);
    }
    else if (paramID == ultraglitch::params::BufferStutter_Mix)
    {
        setMix(value); // Mix parameter is 0-1, so no division by 100
    }
}

void BufferStutter::setStutterRate(float rate)
{
    stutterRate_ = ultraglitch::dsp::clamp(rate, 1.0f, 16.0f); // tasq.md range
    updateInternalState();
}

void BufferStutter::setStutterLength(float lengthMs)
{
    sliceLengthMs_ = ultraglitch::dsp::clamp(lengthMs, 10.0f, 500.0f); // tasq.md range
    updateInternalState();
}

void BufferStutter::updateInternalState()
{
    float triggersPerSecond = stutterRate_; // Simplified interpretation for now
    if (triggersPerSecond <= 0.0f) triggersPerSecond = 1.0f;

    triggerIntervalSamples_ = static_cast<float>(currentSampleRate_) / triggersPerSecond;
    if (triggerIntervalSamples_ < 1.0f) triggerIntervalSamples_ = 1.0f;

    sliceLengthSamples_ = static_cast<int>((sliceLengthMs_ / 1000.0f) * currentSampleRate_);
    sliceLengthSamples_ = juce::jmax(1, sliceLengthSamples_);
}

void BufferStutter::triggerNewSlice()
{
    if (activeSlicesCount_ >= MAX_ACTIVE_SLICES)
    {
        // Pool is full, cannot add new slice (real-time safe: no reallocation)
        return;
    }

    // Find an inactive slot in the pool
    StutterSlice* newSlice = nullptr;
    for (int i = 0; i < MAX_ACTIVE_SLICES; ++i)
    {
        if (!activeSlicesPool_[i].isActive)
        {
            newSlice = &activeSlicesPool_[i];
            activeSlicesCount_++;
            break;
        }
    }

    if (newSlice)
    {
        // Capture a slice from the circular buffer
        int actualSliceLengthSamples = ultraglitch::dsp::clamp(sliceLengthSamples_, 1, bufferDurationSamples_);

        // Start of the slice will be from current writePosition_ - actualSliceLengthSamples
        int startReadPosition = (writePosition_ - actualSliceLengthSamples + bufferDurationSamples_) % bufferDurationSamples_;

        newSlice->startSample = startReadPosition;
        newSlice->lengthSamples = actualSliceLengthSamples;
        newSlice->currentPosition = 0;
        newSlice->isActive = true;
        newSlice->gain = 1.0f;
        newSlice->fadeSamples = juce::jmin(ultraglitch::dsp::CROSSFADE_SAMPLES, actualSliceLengthSamples / 4);
    }
}

} // namespace ultraglitch::dsp
