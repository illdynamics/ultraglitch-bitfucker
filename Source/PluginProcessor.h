#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h> // For juce::Timer
#include "Parameters/PluginParameters.h"
#include "DSP/EffectChain.h"

// Forward declarations
class UltraGlitchAudioProcessorEditor;

class UltraGlitchAudioProcessor : public juce::AudioProcessor,
                                  public juce::Timer // Inherit from juce::Timer
{
public:
    UltraGlitchAudioProcessor();
    ~UltraGlitchAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Timer callback for ChaosController randomization
    void timerCallback() override;

    PluginParameters& getPluginParameters() { return plugin_parameters_; }
    ultraglitch::dsp::EffectChain& getEffectChain() { return effect_chain_; }

private:
    PluginParameters plugin_parameters_;
    ultraglitch::dsp::EffectChain effect_chain_;

    // Private helper methods (may be moved to .cpp later)
    void initializeEffectChain();
    void updateEffectChainParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UltraGlitchAudioProcessor)
};

#endif