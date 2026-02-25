#include "MainPanel.h"
#include "../Common/PluginConfig.h"
#include "../Common/ParameterIDs.h"
#include "Components/EffectModule.h"
#include "Components/GlitchKnob.h"
#include "Components/ChaosButton.h"

namespace ultraglitch::gui
{

MainPanel::MainPanel(PluginParameters& pluginParams)
    : pluginParameters_(pluginParams)
{
    setOpaque(false);
    createUiComponents();
    layoutComponents();
}

void MainPanel::paint(juce::Graphics& /*g*/)
{
    // Transparent: PluginEditor paints the background image.
}

void MainPanel::resized()
{
    layoutComponents();
}

EffectModule* MainPanel::getEffectModule(int index)
{
    if (index >= 0 && index < static_cast<int>(effectModules_.size()))
        return effectModules_[index].get();
    return nullptr;
}

void MainPanel::createUiComponents()
{
    // Header label REMOVED (was drawing the green "UltraGlitch BitFucker")

    // Global Gain Knob
    globalGainKnob_ = std::make_unique<ultraglitch::gui::GlitchKnob>("Output Gain");
    globalGainKnob_->setRange(0.0f, 2.0f, 0.01f);
    globalGainKnob_->setValue(1.0f);
    addAndMakeVisible(*globalGainKnob_);

    // Chaos Button (now ghost-painted by ChaosButton.cpp)
    chaosButton_ = std::make_unique<ultraglitch::gui::ChaosButton>("CHAOS MODE");
    addAndMakeVisible(*chaosButton_);

    createEffectModules();
}

void MainPanel::createEffectModules()
{
    effectModules_.push_back(std::make_unique<EffectModule>(
        "BitCrusher", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::BitCrusher_Enabled,
            ultraglitch::params::BitCrusher_BitDepth,
            ultraglitch::params::BitCrusher_SampleRateDiv,
            ultraglitch::params::BitCrusher_Mix
        }));

    effectModules_.push_back(std::make_unique<EffectModule>(
        "BufferStutter", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::BufferStutter_Enabled,
            ultraglitch::params::BufferStutter_Rate,
            ultraglitch::params::BufferStutter_Length,
            ultraglitch::params::BufferStutter_Mix
        }));

    effectModules_.push_back(std::make_unique<EffectModule>(
        "PitchDrift", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::PitchDrift_Enabled,
            ultraglitch::params::PitchDrift_Amount,
            ultraglitch::params::PitchDrift_Speed,
            ultraglitch::params::PitchDrift_Mix
        }));

    effectModules_.push_back(std::make_unique<EffectModule>(
        "ReverseSlice", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::ReverseSlice_Enabled,
            ultraglitch::params::ReverseSlice_Interval,
            ultraglitch::params::ReverseSlice_Chance,
            ultraglitch::params::ReverseSlice_Mix
        }));

    effectModules_.push_back(std::make_unique<EffectModule>(
        "SliceRearrange", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::SliceRearrange_Enabled,
            ultraglitch::params::SliceRearrange_SliceCount,
            ultraglitch::params::SliceRearrange_Randomize,
            ultraglitch::params::SliceRearrange_Mix
        }));

    effectModules_.push_back(std::make_unique<EffectModule>(
        "WeirdFlanger", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::WeirdFlanger_Enabled,
            ultraglitch::params::WeirdFlanger_Rate,
            ultraglitch::params::WeirdFlanger_Depth,
            ultraglitch::params::WeirdFlanger_Feedback,
            ultraglitch::params::WeirdFlanger_Mix
        }));

    effectModules_.push_back(std::make_unique<EffectModule>(
        "Chaos", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::Global_ChaosMode,
            ultraglitch::params::ChaosController_Speed,
            ultraglitch::params::ChaosController_Intensity
        }));

    for (auto& module : effectModules_)
        addAndMakeVisible(*module);
}

void MainPanel::layoutComponents()
{
    auto bounds = getLocalBounds();

    // No header reserved anymore

    // Footer area (gain knob + chaos)
    auto footerBounds = bounds.removeFromBottom(bounds.getHeight() * 0.2f);
    footerBounds.reduce(padding_, padding_);

    auto gainBounds =
    footerBounds.removeFromLeft(footerBounds.getWidth() / 2).reduced(padding_);

    int gainDiameter = juce::jmin(gainBounds.getWidth(), gainBounds.getHeight());
    int gainOffset   = gainDiameter / 4;   // quarter diameter
    
    gainBounds.setY(gainBounds.getY() + gainOffset);
    
    globalGainKnob_->setBounds(gainBounds);

    chaosButton_->setBounds(footerBounds.reduced(padding_));

    // Modules fill the rest
    auto modulesBounds = bounds;
    modulesBounds.reduce(padding_, padding_);

    int currentX = modulesBounds.getX();
    int currentY = modulesBounds.getY();
    const int moduleSpacingX = padding_;
    const int moduleSpacingY = padding_;
    const int modulesPerRow = 3;

    moduleWidth_ =
        (modulesBounds.getWidth() - (modulesPerRow - 1) * moduleSpacingX)
        / modulesPerRow;

    moduleHeight_ =
        (modulesBounds.getHeight()
         - (int)(std::ceil(numEffectModules_ / (float)modulesPerRow) - 1)
           * moduleSpacingY)
        / (int)std::ceil(numEffectModules_ / (float)modulesPerRow);

    for (int i = 0; i < numEffectModules_; ++i)
    {
        effectModules_[i]->setBounds(currentX, currentY,
                                     moduleWidth_, moduleHeight_);

        currentX += (moduleWidth_ + moduleSpacingX);

        if ((i + 1) % modulesPerRow == 0)
        {
            currentX = modulesBounds.getX();
            currentY += (moduleHeight_ + moduleSpacingY);
        }
    }
}

} // namespace ultraglitch::gui
