#include "ReverseSlice.h"
#include "../../Common/DSPUtils.h"
#include "../../Common/ParameterIDs.h"

namespace ultraglitch::dsp
{

ReverseSlice::ReverseSlice()
{
    setEnabled(false);
    setMix(0.0f);

    internalBuffer_.setSize(2, MAX_SLICE_BUFFER_SAMPLES, false, true, true);
}

void ReverseSlice::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate_ = sampleRate;
    currentMaxBlockSize_ = maxBlockSize;

    dryBuffer_.setSize(2, maxBlockSize);

    internalBuffer_.clear();

    processedSliceBuffer_.setSize(2, MAX_SLICE_BUFFER_SAMPLES, false, true, true);
    pendingSliceBuffer_.setSize(2, MAX_SLICE_BUFFER_SAMPLES, false, true, true);

    updateInternalState();
    reset();
}

void ReverseSlice::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin(buffer.getNumChannels(), 2);

    // Guard: host may deliver blocks larger than maxBlockSize from prepare()
    if (numSamples > dryBuffer_.getNumSamples() || numChannels > dryBuffer_.getNumChannels())
        dryBuffer_.setSize(juce::jmax(numChannels, 2), numSamples, false, false, true);

    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer_.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    const float currentMix = getMix();

    for (int i = 0; i < numSamples; ++i)
    {
        // Build slice
        if (samplesSinceLastSlice_ < sliceIntervalSamples_)
        {
            for (int ch = 0; ch < numChannels; ++ch)
                internalBuffer_.setSample(ch, samplesSinceLastSlice_,
                                          dryBuffer_.getSample(ch, i));

            ++samplesSinceLastSlice_;
        }

        // Slice completed
        if (samplesSinceLastSlice_ >= sliceIntervalSamples_)
        {
            juce::AudioBuffer<float>* target = nullptr;

            if (!isPlayingSlice_)
            {
                target = &processedSliceBuffer_;
            }
            else
            {
                target = &pendingSliceBuffer_;
                pendingSliceReady_ = true;
            }

            for (int ch = 0; ch < numChannels; ++ch)
                target->copyFrom(ch, 0, internalBuffer_, ch, 0, sliceIntervalSamples_);

            if (randomGenerator_.nextFloat() < reverseChance_)
            {
                reverseAudioSegment(*target, 0, sliceIntervalSamples_);
                ultraglitch::dsp::apply_slice_crossfade(*target, 0, sliceIntervalSamples_);
            }

            samplesSinceLastSlice_ = 0;
            internalBuffer_.clear();

            if (!isPlayingSlice_)
            {
                isPlayingSlice_ = true;
                playheadInSlice_ = 0;
            }
        }

        // Output
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float dry = dryBuffer_.getSample(ch, i);
            float wet = dry;

            if (isPlayingSlice_)
            {
                int safeIdx = juce::jlimit(0, sliceIntervalSamples_ - 1, playheadInSlice_);
                wet = processedSliceBuffer_.getSample(ch, safeIdx);
            }

            float out =
                (currentMix <= 0.0f) ? dry :
                (currentMix >= 1.0f) ? wet :
                ultraglitch::dsp::mix(dry, wet, currentMix);

            buffer.setSample(ch, i, out);
        }

        // Advance playback
        if (isPlayingSlice_)
        {
            ++playheadInSlice_;

            if (playheadInSlice_ >= sliceIntervalSamples_)
            {
                isPlayingSlice_ = false;
                playheadInSlice_ = 0;

                if (pendingSliceReady_)
                {
                    // RT-safe content swap (no allocations)
                    const int channels = processedSliceBuffer_.getNumChannels();
                    const int samples  = processedSliceBuffer_.getNumSamples();

                    for (int ch = 0; ch < channels; ++ch)
                    {
                        float* a = processedSliceBuffer_.getWritePointer(ch);
                        float* b = pendingSliceBuffer_.getWritePointer(ch);

                        for (int s = 0; s < samples; ++s)
                            std::swap(a[s], b[s]);
                    }

                    pendingSliceReady_ = false;
                    isPlayingSlice_ = true;
                    playheadInSlice_ = 0;
                }
            }
        }
    }
}

void ReverseSlice::reset()
{
    internalBuffer_.clear();
    processedSliceBuffer_.clear();
    pendingSliceBuffer_.clear();

    samplesSinceLastSlice_ = 0;
    isPlayingSlice_ = false;
    pendingSliceReady_ = false;
    playheadInSlice_ = 0;

    randomGenerator_.setSeed(juce::Time::currentTimeMillis());
}

void ReverseSlice::setParameterValue(const juce::String& paramID, float value)
{
    if (paramID == ultraglitch::params::ReverseSlice_Enabled)
        setEnabled(value > 0.5f);
    else if (paramID == ultraglitch::params::ReverseSlice_Interval)
        setSliceIntervalMs(value);
    else if (paramID == ultraglitch::params::ReverseSlice_Chance)
        setReverseChance(value);
    else if (paramID == ultraglitch::params::ReverseSlice_Mix)
        setMix(value);
}

void ReverseSlice::setSliceIntervalMs(float intervalMs)
{
    sliceIntervalMs_ = ultraglitch::dsp::clamp(intervalMs, 50.0f, 1000.0f);
    updateInternalState();
}

void ReverseSlice::setReverseChance(float chance)
{
    reverseChance_ = ultraglitch::dsp::clamp(chance, 0.0f, 1.0f);
}

void ReverseSlice::updateInternalState()
{
    sliceIntervalSamples_ =
        static_cast<int>((sliceIntervalMs_ / 1000.0f) * currentSampleRate_);

    sliceIntervalSamples_ = juce::jmax(1, sliceIntervalSamples_);
    sliceIntervalSamples_ =
        juce::jmin(sliceIntervalSamples_, MAX_SLICE_BUFFER_SAMPLES);
}

void ReverseSlice::reverseAudioSegment(juce::AudioBuffer<float>& buffer,
                                       int startSample,
                                       int numSamples)
{
    const int numChannels = buffer.getNumChannels();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples / 2; ++i)
        {
            int a = startSample + i;
            int b = startSample + numSamples - 1 - i;
            std::swap(data[a], data[b]);
        }
    }
}

}
