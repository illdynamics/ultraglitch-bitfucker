#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace ultraglitch::gui
{
class ChaosButton : public juce::TextButton
{
public:
    explicit ChaosButton(const juce::String& name);
    ~ChaosButton() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChaosButton)
};
} // namespace ultraglitch::gui
