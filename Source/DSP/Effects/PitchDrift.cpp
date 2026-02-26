#include "PitchDrift.h"
#include "../../Common/DSPUtils.h"
#include "../../Common/ParameterIDs.h"

namespace ultraglitch::dsp
{

PitchDrift::PitchDrift()
    : delayLine_(MAX_DELAY_SAMPLES)
{
    // Initialize base class members
    setEnabled(false); // Start disabled
    setMix(1.0f);      // Default to 100% wet
}

void PitchDrift::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate_ = sampleRate;
    currentMaxBlockSize_ = maxBlockSize;
    
    // Prepare delay line with current sample rate and max block size
    juce::dsp::ProcessSpec spec = { sampleRate, static_cast<juce::uint32>(maxBlockSize), 2 }; // Assuming stereo
    delayLine_.prepare(spec);
    
    dryBuffer_.setSize(2, maxBlockSize); // Preallocate for stereo
    
    updateLFO(); // Calculate LFO increment based on current speed and sample rate
    reset();
}

void PitchDrift::process(juce::AudioBuffer<float>& buffer)
{
    // The EffectChain handles isEnabled() check, so we process if we get here.
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Guard: host may deliver blocks larger than maxBlockSize from prepare()
    if (numSamples > dryBuffer_.getNumSamples() || numChannels > dryBuffer_.getNumChannels())
        dryBuffer_.setSize(juce::jmax(numChannels, 2), numSamples, false, false, true);

    // Copy input to dryBuffer_ for mixing later
    for(int ch = 0; ch < numChannels; ++ch)
        dryBuffer_.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx)
    {
        // Calculate current LFO value (0.0 to 1.0, then scale by amount)
        float lfoValue = (std::sin(static_cast<float>(lfoPhase_) * juce::MathConstants<float>::twoPi) * 0.5f + 0.5f); // Sine LFO 0-1

        // Convert cents to a pitch ratio (2^(cents/1200))
        // amountCents_ is +/- deviation. lfoValue is 0-1.
        // So total cents variation is +/- amountCents_
        float pitchDeviationCents = (lfoValue * 2.0f - 1.0f) * amountCents_;
        float pitchRatio = std::pow(2.0f, pitchDeviationCents / 1200.0f);

        // Modulate delay time to achieve pitch shift
        // Base delay for static pitch (e.g., 20ms)
        const float baseDelayMs = 20.0f;
        const float baseDelaySamples = (baseDelayMs / 1000.0f) * static_cast<float>(currentSampleRate_);

        // Modulated delay time to achieve pitch shift: delay time = base_delay / pitch_ratio
        // When pitchRatio > 1 (higher pitch), delay time decreases.
        // When pitchRatio < 1 (lower pitch), delay time increases.
        float modulatedDelaySamples = baseDelaySamples / pitchRatio;

        // Clamp delay to within reasonable bounds to prevent issues
        modulatedDelaySamples = ultraglitch::dsp::clamp(modulatedDelaySamples, 1.0f, static_cast<float>(MAX_DELAY_SAMPLES - 1));
        
        delayLine_.setDelay(modulatedDelaySamples);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            float inputSample = dryBuffer_.getSample(channel, sampleIdx); // Read from dry buffer
            
            // Push sample into delay line, get interpolated sample out
            delayLine_.pushSample(channel, inputSample);
            float processedSample = delayLine_.popSample(channel);
            
            // Apply dry/wet mix from EffectBase
            buffer.setSample(channel, sampleIdx, ultraglitch::dsp::mix(inputSample, processedSample, getMix()));
        }

        // Advance LFO phase
        lfoPhase_ += lfoPhaseIncrement_;
        if (lfoPhase_ >= 1.0)
        {
            lfoPhase_ -= 1.0;
        }
    }
}

void PitchDrift::reset()
{
    delayLine_.reset();
    lfoPhase_ = 0.0;
}

void PitchDrift::setParameterValue(const juce::String& paramID, float value)
{
    if (paramID == ultraglitch::params::PitchDrift_Enabled)
    {
        setEnabled(value > 0.5f);
    }
    else if (paramID == ultraglitch::params::PitchDrift_Amount)
    {
        setAmount(value);
    }
    else if (paramID == ultraglitch::params::PitchDrift_Speed)
    {
        setSpeed(value);
    }
    else if (paramID == ultraglitch::params::PitchDrift_Mix)
    {
        setMix(value); // Param is 0..1, pass directly
    }
}

void PitchDrift::setAmount(float amountCents)
{
    amountCents_ = ultraglitch::dsp::clamp(amountCents, 0.0f, 1200.0f); // Match param definition 0..1200 cents
}

void PitchDrift::setSpeed(float speedHz)
{
    speedHz_ = ultraglitch::dsp::clamp(speedHz, 0.01f, 10.0f); // tasq.md range
    updateLFO(); // Recalculate LFO increment
}

void PitchDrift::updateLFO()
{
    if (currentSampleRate_ > 0)
    {
        lfoPhaseIncrement_ = speedHz_ / currentSampleRate_;
    } else {
        lfoPhaseIncrement_ = 0.0;
    }
}

// float PitchDrift::calculateCurrentPitchShift() // No longer needed, logic moved to process
// {
//    // LFO waveform handling could be added here if a 'waveform' parameter is introduced
//    return 0.0f;
// }

} // namespace ultraglitch::dsp
