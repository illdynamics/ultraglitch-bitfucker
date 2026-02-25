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
    createUiComponents();
    setupParameterBindings();

    setInterceptsMouseClicks(true, true);
    setOpaque(false); // allow parent background/texture through
}

void EffectModule::paint(juce::Graphics& /*g*/)
{
    // Draw nothing here: no fills, no borders.
    // Keeps global background/texture fully visible.
}

void EffectModule::resized()
{
    layoutComponents();
}

std::vector<GlitchKnob*> EffectModule::getParameterKnobs()
{
    std::vector<GlitchKnob*> knobs;
    knobs.reserve(parameterKnobs_.size());

    for (auto& knob : parameterKnobs_)
        knobs.push_back(knob.get());

    return knobs;
}

void EffectModule::createUiComponents()
{
    // No title label (requested)

    powerButton_ = std::make_unique<ultraglitch::gui::PowerButton>("Power");
    addAndMakeVisible(*powerButton_);

    for (size_t i = 1; i < parameterIDs_.size(); ++i)
    {
        auto paramDef = PluginParameters::get_parameter_definitions();
        juce::String paramName;

        for (const auto& def : paramDef)
        {
            if (def.id == parameterIDs_[i])
            {
                paramName = def.name;
                break;
            }
        }

        if (paramName.isEmpty())
            paramName = "Param " + juce::String(i);

        // GlitchKnob now renders no label/text anyway
        auto knob = std::make_unique<ultraglitch::gui::GlitchKnob>(paramName);
        parameterKnobs_.push_back(std::move(knob));
        addAndMakeVisible(*parameterKnobs_.back());
    }

    layoutComponents();
}

void EffectModule::layoutComponents()
{
    auto bounds = getLocalBounds();
    const int padding = 6;

    // ---- POWER BUTTON: square, sized to match your chopped slot ----
    // Tune this if your artwork slot is slightly different.
    const int powerSize = 52; // <- try 64..76 if you want tighter fit
    const int verticalOffset = 32;   // half square size

    powerButton_->setBounds(
        bounds.getX() + padding,
        bounds.getY() + padding + verticalOffset,
        powerSize,
        powerSize
    );

    // ---- KNOB AREA: everything below the power button ----
    auto knobArea = bounds.withTrimmedTop(powerButton_->getBottom() + padding);
    knobArea.reduce(padding, padding);

    const int numKnobs = static_cast<int>(parameterKnobs_.size());
    if (numKnobs <= 0)
        return;

    const int knobWidth = (knobArea.getWidth() - (numKnobs - 1) * padding) / numKnobs;
    const int knobHeight = knobArea.getHeight();

    for (int i = 0; i < numKnobs; ++i)
    {
        int knobDiameter = juce::jmin(knobWidth, knobHeight);
        int knobOffset   = knobDiameter / 2;
        
        parameterKnobs_[i]->setBounds(
            knobArea.getX() + i * (knobWidth + padding),
            knobArea.getY() + knobOffset,
            knobWidth,
            knobHeight
        );
    }
}

void EffectModule::setupParameterBindings()
{
    auto& apvts = pluginParameters_.get_value_tree_state();

    if (!parameterIDs_.empty())
    {
        powerButtonAttachment_ =
            std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                apvts, parameterIDs_[0], *powerButton_);
    }

    for (size_t i = 1; i < parameterIDs_.size(); ++i)
    {
        if (parameterKnobs_.size() >= i)
        {
            knobAttachments_.push_back(
                std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                    apvts,
                    parameterIDs_[i],
                    *parameterKnobs_[i - 1]
                )
            );
        }
    }
}

} // namespace ultraglitch::gui
