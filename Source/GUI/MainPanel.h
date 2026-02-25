#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
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
    explicit MainPanel(PluginParameters& pluginParams);
    ~MainPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    ChaosButton* getChaosButton() { return chaosButton_.get(); }
    GlitchKnob* getGlobalGainKnob() { return globalGainKnob_.get(); }
    EffectModule* getEffectModule(int index);

private:
    PluginParameters& pluginParameters_;

    // UI Components (header label removed)
    std::unique_ptr<ultraglitch::gui::ChaosButton> chaosButton_;
    std::unique_ptr<ultraglitch::gui::GlitchKnob> globalGainKnob_;
    std::vector<std::unique_ptr<ultraglitch::gui::EffectModule>> effectModules_;

    void createUiComponents();
    void layoutComponents();
    void createEffectModules();

    // Layout constants
    int moduleWidth_ = 150;
    int moduleHeight_ = 200;
    static constexpr int padding_ = 10;
    static constexpr int numEffectModules_ = 7;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPanel)
};
} // namespace ultraglitch::gui