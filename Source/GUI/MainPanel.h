#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel/GlitchLookAndFeel.h"
#include "../Parameters/PluginParameters.h"
#include "../Common/ParameterIDs.h"
#include "Components/EffectModule.h"
#include "Components/GlitchKnob.h"
#include "Components/ChaosButton.h"
#include <vector>

namespace ultraglitch::gui
{
class MainPanel : public juce::Component
{
public:
    explicit MainPanel(PluginParameters& pluginParams); // Takes PluginParameters for EffectModules
    ~MainPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Getters for components, needed by PluginEditor for attachments
    ChaosButton* getChaosButton() { return chaosButton_.get(); }
    GlitchKnob* getGlobalGainKnob() { return globalGainKnob_.get(); }
    EffectModule* getEffectModule(int index);
    
private:
    PluginParameters& pluginParameters_; // Reference to the plugin parameters
    
    // UI Components
    std::unique_ptr<juce::Label> headerLabel_;
    std::unique_ptr<ultraglitch::gui::ChaosButton> chaosButton_;
    std::unique_ptr<ultraglitch::gui::GlitchKnob> globalGainKnob_;
    std::vector<std::unique_ptr<ultraglitch::gui::EffectModule>> effectModules_;
    
    // Helper methods
    void createUiComponents();
    void layoutComponents();
    void createEffectModules(); // Helper to create all effect modules
    
    // Constants
    static constexpr int headerHeight_ = 60;
    int moduleWidth_ = 150;
    int moduleHeight_ = 200;
    static constexpr int padding_ = 10;
    static constexpr int numEffectModules_ = 7;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPanel)
};
} // namespace ultraglitch::gui