#include "EffectModule.h"
#include "../LookAndFeel/GlitchLookAndFeel.h"
#include "../../Common/PluginConfig.h"

namespace ultraglitch::gui
{

EffectModule::EffectModule(const juce::String& moduleName,
                           PluginParameters& pluginParams,
                           const std::vector<juce::String>& parameterIDs)
    : moduleName_(moduleName),
      pluginParameters_(pluginParams),
      parameterIDs_(parameterIDs)
{
    // LookAndFeel is set globally by PluginEditor; no need to set per-component

    createUiComponents();
    setupParameterBindings();
    layoutComponents(); // Initial layout
    
    setInterceptsMouseClicks(true, true); // Allow interaction with this component and its children
}

void EffectModule::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto& lf = getLookAndFeel();

    // Draw background
    g.setColour(lf.findColour(juce::ResizableWindow::backgroundColourId).darker(0.1f));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Draw border
    g.setColour(lf.findColour(juce::Slider::rotarySliderOutlineColourId).darker(0.1f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void EffectModule::resized()
{
    layoutComponents();
}

std::vector<GlitchKnob*> EffectModule::getParameterKnobs()
{
    std::vector<GlitchKnob*> knobs;
    for (auto& knob : parameterKnobs_)
        knobs.push_back(knob.get());
    return knobs;
}

void EffectModule::createUiComponents()
{
    // Title Label
    titleLabel_ = std::make_unique<juce::Label>(moduleName_, moduleName_);
    titleLabel_->setJustificationType(juce::Justification::centred);
    titleLabel_->setColour(juce::Label::textColourId, findColour(juce::Label::textColourId));
    titleLabel_->setFont(juce::Font(juce::FontOptions(16.0f)).boldened());
    addAndMakeVisible(*titleLabel_);

    // Power Button (assuming first parameterID is the enable/bypass for this module)
    powerButton_ = std::make_unique<ultraglitch::gui::PowerButton>("Power");
    addAndMakeVisible(*powerButton_);

    // Create GlitchKnobs for remaining parameters
    // parameterIDs_ should be ordered: [enable, param1, param2, mix]
    for (size_t i = 1; i < parameterIDs_.size(); ++i) // Start from 1 to skip enable param
    {
        // Get parameter name from PluginParameters
        auto paramDef = PluginParameters::get_parameter_definitions();
        juce::String paramName = "";
        for (const auto& def : paramDef) {
            if (def.id == parameterIDs_[i]) {
                paramName = def.name;
                break;
            }
        }
        if (paramName.isEmpty()) paramName = "Param " + juce::String(i); // Fallback
        
        auto knob = std::make_unique<ultraglitch::gui::GlitchKnob>(paramName);
        parameterKnobs_.push_back(std::move(knob));
        addAndMakeVisible(*parameterKnobs_.back());
    }
}

void EffectModule::layoutComponents()
{
    auto bounds = getLocalBounds();
    auto padding = 5;

    // Title Label at the top
    titleLabel_->setBounds(padding, padding, bounds.getWidth() - 2 * padding, 20);

    // Power Button on the left
    powerButton_->setBounds(padding, titleLabel_->getBottom() + padding, 50, 50);

    // Knobs arranged in a row/grid
    auto knobArea = bounds.removeFromBottom(bounds.getHeight() - powerButton_->getBottom() - padding);
    knobArea.reduce(padding, padding);

    int numKnobs = static_cast<int>(parameterKnobs_.size());
    if (numKnobs > 0)
    {
        int knobWidth = (knobArea.getWidth() - (numKnobs - 1) * padding) / numKnobs;
        int knobHeight = knobArea.getHeight();

        for (int i = 0; i < numKnobs; ++i)
        {
            parameterKnobs_[i]->setBounds(knobArea.getX() + i * (knobWidth + padding),
                                          knobArea.getY(),
                                          knobWidth, knobHeight);
        }
    }
}

void EffectModule::setupParameterBindings()
{
    auto& apvts = pluginParameters_.get_value_tree_state();

    // Bind Power Button (first parameter in the list)
    if (!parameterIDs_.empty())
    {
        powerButtonAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, parameterIDs_[0], *powerButton_
        );
    }

    // Bind GlitchKnobs for remaining parameters
    for (size_t i = 1; i < parameterIDs_.size(); ++i)
    {
        if (parameterKnobs_.size() >= (i)) // Knob indices match parameterIDs_[1] onwards
        {
            knobAttachments_.push_back(
                std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                    apvts, parameterIDs_[i], *parameterKnobs_[i-1] // i-1 because knob vector is 0-indexed
                )
            );
        }
    }
}

} // namespace ultraglitch::gui