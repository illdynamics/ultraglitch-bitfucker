#pragma once

#include "../EffectBase.h" // Points to ultraglitch::dsp::EffectBase
#include "../../Common/DSPUtils.h" // Points to ultraglitch::dsp::DSPUtils
#include "../../Common/ParameterIDs.h" // For parameter IDs
#include "../../Parameters/PluginParameters.h" // To access other plugin parameters
#include <random>

namespace ultraglitch::dsp
{
class ChaosController : public ultraglitch::dsp::EffectBase
{
public:
    // ChaosController needs access to all plugin parameters to randomize them.
    explicit ChaosController(PluginParameters* pluginParameters);
    ~ChaosController() override = default;

    void prepare(double sampleRate, int maxBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override; // Does not process audio, but triggers updates
    void reset() override;

    // Parameter setter from PluginParameters/EffectChain
    void setParameterValue(const juce::String& paramID, float value) override;

    // Specific parameter setters (internal, might be called by setParameterValue)
    void setSpeed(float speed); // chaosSpeed (changes per second)
    void setIntensity(float intensity); // chaosIntensity (range of randomization)

    // Public method to trigger randomization (called from message thread)
    void randomizeParameters();

    // Methods to manage randomization trigger from audio thread
    bool shouldTriggerRandomization() { return shouldTriggerRandomization_.exchange(false); } // Get and clear
    void clearRandomizationTrigger() { shouldTriggerRandomization_.store(false); } // Explicitly clear

    [[nodiscard]] juce::String getName() const override { return "ChaosController"; }

private:
    PluginParameters* pluginParameters_ = nullptr; // Raw pointer, owned by PluginProcessor

    // Parameters
    float speed_ = 1.0f; // Rate of randomization in Hz (tasq.md: 1/8 note)
    float intensity_ = 0.5f; // Amount of randomization (0.0 to 1.0)

    double currentSampleRate_ = 0.0;
    int currentMaxBlockSize_ = 0;

    // Internal state for triggering randomization
    int samplesCounter_ = 0;
    int updateIntervalSamples_ = 0; // Number of samples between randomization triggers

    std::mt19937 randomGenerator_; // Standard Mersenne Twister engine
    juce::Random juceRandomGenerator_; // For convenient JUCE random functions

    std::atomic<bool> shouldTriggerRandomization_ = false; // Flag to signal randomization from audio thread

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosController)
};
} // namespace ultraglitch::dsp