#pragma once

#include <juce_core/juce_core.h> // For juce::jlimit
#include <juce_audio_basics/juce_audio_basics.h> // For juce::AudioBuffer
#include <atomic>

namespace ultraglitch::dsp {
    class EffectBase {
    public:
        virtual ~EffectBase() = default;
        virtual void prepare(double sampleRate, int maxBlockSize) = 0;
        virtual void process(juce::AudioBuffer<float>& buffer) = 0;
        virtual void reset() = 0;
        
        // Concrete methods, declarations remain here, implementations move to .cpp
        void setEnabled(bool enabled);
        [[nodiscard]] bool isEnabled() const;
        
        void setMix(float mix);
        [[nodiscard]] float getMix() const;

        // New virtual method for getting effect name
        virtual juce::String getName() const = 0;

        // New virtual method for setting parameters by ID, with default no-op
        virtual void setParameterValue(const juce::String& paramID, float value) { juce::ignoreUnused(paramID, value); }
        
    protected:
        std::atomic<bool> enabled{false};
        std::atomic<float> dryWetMix{1.0f};
        double currentSampleRate{44100.0};
        int currentBlockSize{512};
    };
}