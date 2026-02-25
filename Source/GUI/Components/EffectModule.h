#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../../Parameters/PluginParameters.h"
#include "../../Common/ParameterIDs.h"
#include "GlitchKnob.h"
#include "PowerButton.h"
#include <vector>

namespace ultraglitch::gui
{
class EffectModule : public juce::Component
{
public:
    // Constructor takes module name, PluginParameters reference, and a list of parameter IDs for this module
    EffectModule(const juce::String& moduleName,
                 PluginParameters& pluginParams,
                 const std::vector<juce::String>& parameterIDs);
    ~EffectModule() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Getters for components, needed by MainPanel/PluginEditor for attachments
    PowerButton* getPowerButton() { return powerButton_.get(); }
    std::vector<GlitchKnob*> getParameterKnobs();
    
private:
    juce::String moduleName_; // Renamed
    PluginParameters& pluginParameters_; // Reference to the plugin parameters
    std::vector<juce::String> parameterIDs_; // List of parameter IDs this module manages
    
    std::unique_ptr<ultraglitch::gui::PowerButton> powerButton_; // Renamed
    std::unique_ptr<juce::Label> titleLabel_; // To display module name
    std::vector<std::unique_ptr<ultraglitch::gui::GlitchKnob>> parameterKnobs_; // Renamed
    // std::vector<std::unique_ptr<juce::Label>> parameterLabels; // Labels can be inside GlitchKnob

    // Attachments for this module's parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerButtonAttachment_;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> knobAttachments_;

    void createUiComponents(); // Renamed
    void layoutComponents(); // Renamed
    void setupParameterBindings(); // Renamed
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectModule)
};
} // namespace ultraglitch::gui