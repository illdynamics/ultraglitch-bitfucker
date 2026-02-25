#include "ChaosController.h"
#include "../../Common/DSPUtils.h"
#include "../../Common/ParameterIDs.h"
#include "../../Parameters/PluginParameters.h" // Needed for PluginParameters definition
#include <random> // For std::mt19937

namespace ultraglitch::dsp
{

ChaosController::ChaosController(PluginParameters* pluginParameters)
    : pluginParameters_(pluginParameters),
      randomGenerator_(std::random_device{}()), // Initialize std::mt19937
      juceRandomGenerator_(juce::Time::currentTimeMillis()) // Initialize juce::Random
{
    // Initialize base class members
    setEnabled(false); // Start disabled
    setMix(0.0f);      // Chaos controller doesn't process audio, so mix is irrelevant
}

void ChaosController::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate_ = sampleRate;
    currentMaxBlockSize_ = maxBlockSize;
    
    // Calculate initial update interval based on speed
    setSpeed(speed_); // Call setter to recalculate updateIntervalSamples_
    reset();
}

void ChaosController::process(juce::AudioBuffer<float>& buffer)
{
    juce::ignoreUnused(buffer); // This effect does not process audio directly

    if (!isEnabled()) // Check if chaos mode is enabled
        return;

    const int numSamples = buffer.getNumSamples();
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        samplesCounter_++;
        
        if (samplesCounter_ >= updateIntervalSamples_)
        {
            shouldTriggerRandomization_.store(true); // Signal randomization is needed
            samplesCounter_ = 0;
            // Recalculate interval in case speed changed
            setSpeed(speed_); // This will update updateIntervalSamples_
        }
    }
}

void ChaosController::reset()
{
    samplesCounter_ = 0;
    shouldTriggerRandomization_.store(false); // Clear any pending randomization trigger
}

void ChaosController::setParameterValue(const juce::String& paramID, float value)
{
    if (paramID == ultraglitch::params::Global_ChaosMode)
    {
        bool wasEnabled = isEnabled();
        setEnabled(value > 0.5f);
        if (wasEnabled && !isEnabled())
        {
            // Chaos disabled — stop any pending triggers and reset counter
            samplesCounter_ = 0;
            shouldTriggerRandomization_.store(false);
        }
    }
    else if (paramID == ultraglitch::params::ChaosController_Speed)
    {
        setSpeed(value);
    }
    else if (paramID == ultraglitch::params::ChaosController_Intensity)
    {
        setIntensity(value / 100.0f); // Param is 0..100%, DSP expects 0..1
    }
}

void ChaosController::setSpeed(float speed)
{
    speed_ = ultraglitch::dsp::clamp(speed, 0.01f, 10.0f); // tasq.md range 0.01-10.0 Hz
    if (currentSampleRate_ > 0 && speed_ > 0.0f)
    {
        updateIntervalSamples_ = static_cast<int>(currentSampleRate_ / speed_);
    } else {
        updateIntervalSamples_ = static_cast<int>(currentSampleRate_); // Default to 1 second if speed is 0 or no sample rate
    }
    updateIntervalSamples_ = juce::jmax(1, updateIntervalSamples_); // Minimum 1 sample interval
}

void ChaosController::setIntensity(float intensity)
{
    intensity_ = ultraglitch::dsp::clamp(intensity, 0.0f, 1.0f); // 0-1 range
}

void ChaosController::randomizeParameters()
{
    if (!pluginParameters_)
        return;

    const auto& allParameters = pluginParameters_->get_parameter_definitions();

    for (const auto& paramDef : allParameters)
    {
        // Don't randomize ChaosController's own parameters or global gain
        if (paramDef.id == ultraglitch::params::Global_ChaosMode ||
            paramDef.id == ultraglitch::params::ChaosController_Speed ||
            paramDef.id == ultraglitch::params::ChaosController_Intensity ||
            paramDef.id == ultraglitch::params::Global_OutputGain) // Global_Gain is now Global_OutputGain
            continue;

        // Randomly decide if this parameter should be changed based on intensity
        if (juceRandomGenerator_.nextFloat() < intensity_) // Intensity is 0-1, acts as probability
        {
            float minValue = paramDef.minValue;
            float maxValue = paramDef.maxValue;

            float newValue = juceRandomGenerator_.nextFloat() * (maxValue - minValue) + minValue;
            pluginParameters_->set_parameter_value(paramDef.id, newValue);
        }
    }
}

} // namespace ultraglitch::dsp
