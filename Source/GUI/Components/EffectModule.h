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
    EffectModule(const juce::String& moduleName,
                 PluginParameters& pluginParams,
                 const std::vector<juce::String>& parameterIDs);
    ~EffectModule() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    PowerButton* getPowerButton() { return powerButton_.get(); }
    std::vector<GlitchKnob*> getParameterKnobs();

private:
    juce::String moduleName_;
    PluginParameters& pluginParameters_;
    std::vector<juce::String> parameterIDs_;

    std::unique_ptr<ultraglitch::gui::PowerButton> powerButton_;
    std::vector<std::unique_ptr<ultraglitch::gui::GlitchKnob>> parameterKnobs_;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerButtonAttachment_;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> knobAttachments_;

    void createUiComponents();
    void layoutComponents();
    void setupParameterBindings();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectModule)
};
} // namespace ultraglitch::gui