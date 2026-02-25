#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters/PluginParameters.h"
#include "DSP/EffectChain.h"
#include "DSP/EffectBase.h"
#include "Common/ParameterIDs.h"
#include "DSP/Effects/ChaosController.h"

// Effect includes
#include "DSP/Effects/BitCrusher.h"
#include "DSP/Effects/BufferStutter.h"
#include "DSP/Effects/PitchDrift.h"
#include "DSP/Effects/ReverseSlice.h"
#include "DSP/Effects/SliceRearrange.h"
#include "DSP/Effects/WeirdFlanger.h"

//==============================================================================
UltraGlitchAudioProcessor::UltraGlitchAudioProcessor()
    : AudioProcessor (BusesProperties()
                    #if ! JucePlugin_IsMidiEffect
                     #if ! JucePlugin_IsSynth
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     #endif
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                    #endif
                      ),
      plugin_parameters_(*this),
      effect_chain_()
{
    // Warm up the static parameter definitions so the vector is constructed
    // on the main thread before any audio processing begins
    (void) PluginParameters::get_parameter_definitions();
    
    initializeEffectChain();
    startTimerHz(30); // Start timer to check for ChaosController randomization requests
}

UltraGlitchAudioProcessor::~UltraGlitchAudioProcessor()
{
    stopTimer(); // Stop the timer when the processor is destroyed
}

//==============================================================================
const juce::String UltraGlitchAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool UltraGlitchAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool UltraGlitchAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool UltraGlitchAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double UltraGlitchAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int UltraGlitchAudioProcessor::getNumPrograms()
{
    return 1;
}

int UltraGlitchAudioProcessor::getCurrentProgram()
{
    return 0;
}

void UltraGlitchAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String UltraGlitchAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void UltraGlitchAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void UltraGlitchAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    effect_chain_.prepareToPlay(sampleRate, samplesPerBlock);
}

void UltraGlitchAudioProcessor::releaseResources()
{
    effect_chain_.reset();
}

bool UltraGlitchAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void UltraGlitchAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);
    
    // Update effect chain parameters from APVTS
    updateEffectChainParameters();
    
    // Process audio through effect chain
    effect_chain_.process(buffer);
}

//==============================================================================
bool UltraGlitchAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* UltraGlitchAudioProcessor::createEditor()
{
    return new UltraGlitchAudioProcessorEditor (*this, plugin_parameters_);
}

//==============================================================================
void UltraGlitchAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = plugin_parameters_.get_value_tree_state().copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void UltraGlitchAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (plugin_parameters_.get_value_tree_state().state.getType()))
            plugin_parameters_.get_value_tree_state().replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
void UltraGlitchAudioProcessor::timerCallback()
{
    // Check if ChaosController has requested randomization
    for (int i = 0; i < effect_chain_.getNumEffects(); ++i)
    {
        if (auto* effect = effect_chain_.getEffect(i))
        {
            if (effect->getName() == "ChaosController")
            {
                // Cast to ChaosController* to access its specific methods
                if (auto* chaosController = dynamic_cast<ultraglitch::dsp::ChaosController*>(effect))
                {
                    if (chaosController->shouldTriggerRandomization())
                    {
                        chaosController->randomizeParameters();
                        // The shouldTriggerRandomization() getter clears the flag.
                    }
                }
            }
        }
    }
}

//==============================================================================
void UltraGlitchAudioProcessor::initializeEffectChain()
{
    // Add all effects to the effect chain
    effect_chain_.addEffect(std::make_unique<ultraglitch::dsp::BitCrusher>());
    effect_chain_.addEffect(std::make_unique<ultraglitch::dsp::BufferStutter>());
    effect_chain_.addEffect(std::make_unique<ultraglitch::dsp::PitchDrift>());
    effect_chain_.addEffect(std::make_unique<ultraglitch::dsp::ReverseSlice>());
    effect_chain_.addEffect(std::make_unique<ultraglitch::dsp::SliceRearrange>());
    effect_chain_.addEffect(std::make_unique<ultraglitch::dsp::WeirdFlanger>());
    effect_chain_.addEffect(std::make_unique<ultraglitch::dsp::ChaosController>(&plugin_parameters_)); // Pass PluginParameters to ChaosController
}

void UltraGlitchAudioProcessor::updateEffectChainParameters()
{
    // Iterate through all defined parameters and push their current values to the EffectChain
    for (const auto& param_info : PluginParameters::get_parameter_definitions())
    {
        // Get the parameter value from the APVTS managed by plugin_parameters_
        float value = plugin_parameters_.get_parameter_value(param_info.id);
        
        // Pass the parameter ID and its value to the EffectChain
        // The EffectChain will then route this parameter to the relevant effect
        effect_chain_.setParameterValue(param_info.id, value);
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UltraGlitchAudioProcessor();
}