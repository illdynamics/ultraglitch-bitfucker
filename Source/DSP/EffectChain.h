#ifndef EFFECTCHAIN_H
#define EFFECTCHAIN_H

#include <vector>
#include <memory>
#include <juce_audio_basics/juce_audio_basics.h>
#include "EffectBase.h" // Points to ultraglitch::dsp::EffectBase

namespace ultraglitch::dsp
{
class EffectChain
{
public:
    EffectChain();
    ~EffectChain();

    // Effect management
    void addEffect(std::unique_ptr<ultraglitch::dsp::EffectBase> effect);
    void removeEffect(int index);
    void clearEffects();
    
    // Effect control
    void bypassEffect(int index, bool shouldBypass); // Handled by EffectBase::setEnabled
    bool isEffectBypassed(int index) const; // Handled by EffectBase::isEnabled
    void setEffectEnabled(int index, bool enabled); // Handled by EffectBase::setEnabled
    bool isEffectEnabled(int index) const; // Handled by EffectBase::isEnabled
    
    // Parameter management (for individual effects within the chain)
    // This is the method UltraGlitchAudioProcessor will call
    void setParameterValue(const juce::String& paramID, float value);
    // float getEffectParameter(int effect_index, int parameter_id) const; // Removed, not used

    // Processing
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();
    void process(juce::AudioBuffer<float>& buffer); // Aligned with EffectBase::process
    void reset();
    
    // Chain configuration
    void setProcessingOrder(const std::vector<int>& order);
    std::vector<int> getProcessingOrder() const;
    void setGlobalMix(float mix);
    float getGlobalMix() const;
    
    // Information
    int getNumEffects() const;
    ultraglitch::dsp::EffectBase* getEffect(int index) const;
    juce::String getEffectName(int index) const; // Changed to juce::String
    
    // State management
    void saveState(juce::XmlElement& xml) const;
    void loadState(const juce::XmlElement& xml);
    
private:
    struct EffectSlot
    {
        std::unique_ptr<ultraglitch::dsp::EffectBase> effect;
        // bool enabled = true; // Managed by EffectBase
        // bool bypassed = false; // Managed by EffectBase
    };
    
    std::vector<EffectSlot> effects_;
    std::vector<int> processingOrder_; // Renamed
    float globalMix_ = 1.0f; // Renamed
    double sampleRate_ = 44100.0; // Renamed
    int samplesPerBlock_ = 512; // Renamed
    
    juce::AudioBuffer<float> processingBuffer_; // Renamed
    
    void rebuildProcessingOrder(); // Renamed
    void updateEffectPreparation(); // Renamed
};
} // namespace ultraglitch::dsp

#endif // EFFECTCHAIN_H