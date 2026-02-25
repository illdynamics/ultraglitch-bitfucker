#include "EffectChain.h"
#include <algorithm>
#include <cassert>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../Common/ParameterIDs.h" // Needed for parameter IDs
#include "../Common/DSPUtils.h" // Needed for ultraglitch::dsp::clamp and mix (though mix is not used directly yet here)

namespace ultraglitch::dsp
{

EffectChain::EffectChain() = default;

EffectChain::~EffectChain() = default;

void EffectChain::addEffect(std::unique_ptr<ultraglitch::dsp::EffectBase> effect)
{
    EffectSlot slot;
    slot.effect = std::move(effect);
    
    effects_.push_back(std::move(slot));
    rebuildProcessingOrder();
    
    // Prepare the effect if we already have sample rate info
    if (sampleRate_ > 0 && samplesPerBlock_ > 0)
    {
        effects_.back().effect->prepare(sampleRate_, samplesPerBlock_);
    }
}

void EffectChain::removeEffect(int index)
{
    if (index >= 0 && index < static_cast<int>(effects_.size()))
    {
        effects_.erase(effects_.begin() + static_cast<ptrdiff_t>(index));
        rebuildProcessingOrder();
    }
}

void EffectChain::clearEffects()
{
    effects_.clear();
    processingOrder_.clear();
}

void EffectChain::bypassEffect(int index, bool shouldBypass)
{
    if (index >= 0 && index < static_cast<int>(effects_.size()))
    {
        effects_[static_cast<size_t>(index)].effect->setEnabled(!shouldBypass); // Bypass means !enabled
    }
}

bool EffectChain::isEffectBypassed(int index) const
{
    if (index >= 0 && index < static_cast<int>(effects_.size()))
    {
        return !effects_[static_cast<size_t>(index)].effect->isEnabled();
    }
    return false;
}

void EffectChain::setEffectEnabled(int index, bool enabled)
{
    if (index >= 0 && index < static_cast<int>(effects_.size()))
    {
        effects_[static_cast<size_t>(index)].effect->setEnabled(enabled);
        // No need to rebuild — processingOrder_ always contains all effects;
        // enabled/disabled check happens in process()
    }
}

bool EffectChain::isEffectEnabled(int index) const
{
    if (index >= 0 && index < static_cast<int>(effects_.size()))
    {
        return effects_[static_cast<size_t>(index)].effect->isEnabled();
    }
    return false;
}

void EffectChain::setParameterValue(const juce::String& paramID, float value)
{
    if (paramID == ultraglitch::params::Global_OutputGain)
    {
        setGlobalMix(value); // This setter handles clamping
    }
    else if (paramID == ultraglitch::params::Global_ChaosMode)
    {
        // This parameter controls the enabled state of the ChaosController.
        // It needs to be routed specifically to the ChaosController effect.
        for (auto& slot : effects_)
        {
            if (slot.effect && slot.effect->getName() == "ChaosController")
            {
                slot.effect->setParameterValue(paramID, value);
                // Note: ChaosController's setParameterValue should handle its own enabled state.
                break;
            }
        }
    }
    else
    {
        // Route to individual effects. Each effect's setParameterValue will check if the paramID is relevant to it.
        for (auto& slot : effects_)
        {
            if (slot.effect)
            {
                slot.effect->setParameterValue(paramID, value);
            }
        }
    }
}

void EffectChain::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    sampleRate_ = sampleRate;
    samplesPerBlock_ = samplesPerBlock;
    
    // Set up processing buffer size dynamically based on maxBlockSize
    processingBuffer_.setSize(2, samplesPerBlock); // Assuming stereo for now
    processingBuffer_.clear();
    
    for (auto& slot : effects_)
    {
        if (slot.effect)
        {
            slot.effect->prepare(sampleRate, samplesPerBlock);
        }
    }
}

void EffectChain::releaseResources()
{
    for (auto& slot : effects_)
    {
        if (slot.effect)
        {
            slot.effect->reset();
        }
    }
    
    processingBuffer_.setSize(0, 0);
}

void EffectChain::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Ensure processing buffer is the right size (should have been allocated in prepareToPlay)
    // Use avoidReallocating=true to prevent RT allocation
    if (processingBuffer_.getNumChannels() != numChannels || 
        processingBuffer_.getNumSamples() < numSamples)
    {
        processingBuffer_.setSize(numChannels, numSamples, false, false, true);
    }
    
    // Copy input to processing buffer (this will be the 'dry' signal for effects with mix)
    for (int ch = 0; ch < numChannels; ++ch)
    {
        processingBuffer_.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    }
    
    // Process through chain in specified order
    for (auto effectIndex : processingOrder_)
    {
        auto& slot = effects_[static_cast<size_t>(effectIndex)];
        
        if (slot.effect && slot.effect->isEnabled()) // Check if effect is enabled
        {
            slot.effect->process(processingBuffer_);
        }
    }
    
    // Apply global output gain (globalMix_ is now Global_Gain parameter)
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* output = buffer.getWritePointer(ch);
        auto* wet = processingBuffer_.getReadPointer(ch);
        
        for (int i = 0; i < numSamples; ++i)
        {
            // Apply Global_Gain which is stored in globalMix_
            output[i] = wet[i] * globalMix_; 
        }
    }
}

void EffectChain::reset()
{
    for (auto& slot : effects_)
    {
        if (slot.effect)
        {
            slot.effect->reset();
        }
    }
    
    processingBuffer_.clear();
}

void EffectChain::setProcessingOrder(const std::vector<int>& order)
{
    processingOrder_ = order;
    
    // Validate order indices
    for (int index : processingOrder_)
    {
        if (index < 0 || index >= static_cast<int>(effects_.size()))
        {
            rebuildProcessingOrder(); // Revert to default if order is invalid
            return;
        }
    }
}

std::vector<int> EffectChain::getProcessingOrder() const
{
    return processingOrder_;
}

void EffectChain::setGlobalMix(float gain)
{
    // globalMix_ now represents the Global_Gain parameter (linear 0.0-2.0, default 1.0)
    // The name "GlobalMix" might be confusing; it's actually output gain.
    globalMix_ = ultraglitch::dsp::clamp(gain, 0.0f, 2.0f);
}

float EffectChain::getGlobalMix() const
{
    return globalMix_;
}

int EffectChain::getNumEffects() const
{
    return static_cast<int>(effects_.size());
}

ultraglitch::dsp::EffectBase* EffectChain::getEffect(int index) const
{
    if (index >= 0 && index < static_cast<int>(effects_.size()))
    {
        return effects_[static_cast<size_t>(index)].effect.get();
    }
    return nullptr;
}

juce::String EffectChain::getEffectName(int index) const
{
    if (index >= 0 && index < static_cast<int>(effects_.size()) && effects_[static_cast<size_t>(index)].effect)
    {
        return effects_[static_cast<size_t>(index)].effect->getName();
    }
    return juce::String();
}

void EffectChain::saveState(juce::XmlElement& xml) const
{
    xml.setAttribute("GlobalGain", globalMix_);

    auto* effectsElement = xml.createNewChildElement("Effects");
    for (size_t i = 0; i < effects_.size(); ++i)
    {
        auto* effectElement = effectsElement->createNewChildElement("Effect");
        effectElement->setAttribute("Index", static_cast<int>(i));
        
        if (effects_[i].effect)
        {
            effectElement->setAttribute("Name", effects_[i].effect->getName().toStdString());
            effectElement->setAttribute("Enabled", effects_[i].effect->isEnabled());
            // Per-effect parameter state is managed by APVTS — no per-effect save needed
        }
    }
    
    auto* orderElement = xml.createNewChildElement("ProcessingOrder");
    for (int index : processingOrder_)
    {
        auto* indexElement = orderElement->createNewChildElement("Index");
        indexElement->setAttribute("value", index);
    }
}

void EffectChain::loadState(const juce::XmlElement& xml)
{
    globalMix_ = static_cast<float>(xml.getDoubleAttribute("GlobalGain", 1.0));
    
    if (auto* effectsElement = xml.getChildByName("Effects"))
    {
        for (auto* effectElement : effectsElement->getChildIterator())
        {
            if (effectElement->hasTagName("Effect"))
            {
                int index = effectElement->getIntAttribute("Index", -1);
                bool enabled = effectElement->getBoolAttribute("Enabled", true);
                
                if (index >= 0 && index < static_cast<int>(effects_.size()) && effects_[static_cast<size_t>(index)].effect)
                {
                    effects_[static_cast<size_t>(index)].effect->setEnabled(enabled);
                    // Per-effect parameter state is managed by APVTS — no per-effect load needed
                }
            }
        }
    }

    if (auto* orderElement = xml.getChildByName("ProcessingOrder")) // Renamed element
    {
        std::vector<int> order;
        for (auto* indexElement : orderElement->getChildIterator())
        {
            if (indexElement->hasTagName("Index"))
            {
                order.push_back(indexElement->getIntAttribute("value", -1));
            }
        }
        
        if (!order.empty())
        {
            setProcessingOrder(order);
        }
    }
}

void EffectChain::rebuildProcessingOrder()
{
    processingOrder_.clear();
    
    // Include ALL effects in chain order — enabled/disabled check happens in process()
    for (size_t i = 0; i < effects_.size(); ++i)
    {
        if (effects_[i].effect)
        {
            processingOrder_.push_back(static_cast<int>(i));
        }
    }
}

void EffectChain::updateEffectPreparation()
{
    for (auto& slot : effects_)
    {
        if (slot.effect && sampleRate_ > 0 && samplesPerBlock_ > 0)
        {
            slot.effect->prepare(sampleRate_, samplesPerBlock_);
        }
    }
}

} // namespace ultraglitch::dsp
