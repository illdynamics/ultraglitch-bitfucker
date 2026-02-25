#include "MainPanel.h"
#include "LookAndFeel/GlitchLookAndFeel.h"
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
    // LookAndFeel is set globally by PluginEditor; no need to set per-component

    createUiComponents();
    layoutComponents(); // Initial layout
}

void MainPanel::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId)); // Background color from LF
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
    // Header Label
    headerLabel_ = std::make_unique<juce::Label>("headerLabel", "UltraGlitch BitFucker");
    headerLabel_->setFont(juce::Font(juce::FontOptions(24.0f)).boldened());
    headerLabel_->setColour(juce::Label::textColourId, findColour(juce::Label::textColourId));
    headerLabel_->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*headerLabel_);

    // Global Gain Knob
    globalGainKnob_ = std::make_unique<ultraglitch::gui::GlitchKnob>("Output Gain");
    globalGainKnob_->setRange(0.0f, 2.0f, 0.01f); // Linear gain 0.0 to 2.0 as per tasq.md
    globalGainKnob_->setValue(1.0f); // Default 1.0
    addAndMakeVisible(*globalGainKnob_);

    // Chaos Button
    chaosButton_ = std::make_unique<ultraglitch::gui::ChaosButton>("CHAOS MODE");
    addAndMakeVisible(*chaosButton_);

    createEffectModules(); // Create individual effect modules
}

void MainPanel::createEffectModules()
{
    // BitCrusher
    effectModules_.push_back(std::make_unique<EffectModule>(
        "BitCrusher", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::BitCrusher_Enabled,
            ultraglitch::params::BitCrusher_BitDepth,
            ultraglitch::params::BitCrusher_SampleRateDiv,
            ultraglitch::params::BitCrusher_Mix
        }));

    // BufferStutter
    effectModules_.push_back(std::make_unique<EffectModule>(
        "BufferStutter", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::BufferStutter_Enabled,
            ultraglitch::params::BufferStutter_Rate,
            ultraglitch::params::BufferStutter_Length,
            ultraglitch::params::BufferStutter_Mix
        }));

    // PitchDrift
    effectModules_.push_back(std::make_unique<EffectModule>(
        "PitchDrift", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::PitchDrift_Enabled,
            ultraglitch::params::PitchDrift_Amount,
            ultraglitch::params::PitchDrift_Speed,
            ultraglitch::params::PitchDrift_Mix
        }));
    
    // ReverseSlice
    effectModules_.push_back(std::make_unique<EffectModule>(
        "ReverseSlice", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::ReverseSlice_Enabled,
            ultraglitch::params::ReverseSlice_Interval,
            ultraglitch::params::ReverseSlice_Chance,
            ultraglitch::params::ReverseSlice_Mix
        }));

    // SliceRearrange
    effectModules_.push_back(std::make_unique<EffectModule>(
        "SliceRearrange", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::SliceRearrange_Enabled,
            ultraglitch::params::SliceRearrange_SliceCount,
            ultraglitch::params::SliceRearrange_Randomize,
            ultraglitch::params::SliceRearrange_Mix
        }));

    // WeirdFlanger
    effectModules_.push_back(std::make_unique<EffectModule>(
        "WeirdFlanger", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::WeirdFlanger_Enabled,
            ultraglitch::params::WeirdFlanger_Rate,
            ultraglitch::params::WeirdFlanger_Depth,
            ultraglitch::params::WeirdFlanger_Feedback,
            ultraglitch::params::WeirdFlanger_Mix
        }));
    
    // ChaosController - special case, only has enabled, speed, intensity
    effectModules_.push_back(std::make_unique<EffectModule>(
        "Chaos", pluginParameters_,
        std::vector<juce::String>{
            ultraglitch::params::Global_ChaosMode, // This is the global enable for Chaos
            ultraglitch::params::ChaosController_Speed,
            ultraglitch::params::ChaosController_Intensity
        }));

    for (auto& module : effectModules_)
        addAndMakeVisible(*module);
}

void MainPanel::layoutComponents()
{
    auto bounds = getLocalBounds();
    
    // Header
    headerLabel_->setBounds(bounds.removeFromTop(headerHeight_));

    // Controls at the bottom (Global Gain & Chaos Button)
    auto footerBounds = bounds.removeFromBottom(bounds.getHeight() * 0.2f); // 20% of remaining height for footer
    footerBounds.reduce(padding_, padding_);

    globalGainKnob_->setBounds(footerBounds.removeFromLeft(footerBounds.getWidth() / 2).reduced(padding_));
    chaosButton_->setBounds(footerBounds.reduced(padding_));

    // Effect Modules grid (remaining space)
    auto modulesBounds = bounds; // What's left
    modulesBounds.reduce(padding_, padding_);

    int currentX = modulesBounds.getX();
    int currentY = modulesBounds.getY();
    int moduleSpacingX = padding_;
    int moduleSpacingY = padding_;

    int modulesPerRow = 3; // Example: 3 modules per row

    moduleWidth_ = (modulesBounds.getWidth() - (modulesPerRow - 1) * moduleSpacingX) / modulesPerRow;
    moduleHeight_ = (modulesBounds.getHeight() - (int)(std::ceil(numEffectModules_ / (float)modulesPerRow) - 1) * moduleSpacingY) / (int)std::ceil(numEffectModules_ / (float)modulesPerRow);

    for (int i = 0; i < numEffectModules_; ++i)
    {
        effectModules_[i]->setBounds(currentX, currentY, moduleWidth_, moduleHeight_);

        currentX += (moduleWidth_ + moduleSpacingX);
        if ((i + 1) % modulesPerRow == 0) // Move to next row
        {
            currentX = modulesBounds.getX();
            currentY += (moduleHeight_ + moduleSpacingY);
        }
    }
}

} // namespace ultraglitch::gui