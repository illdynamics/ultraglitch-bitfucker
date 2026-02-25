#include "PowerButton.h"
#include "../LookAndFeel/GlitchLookAndFeel.h"

namespace ultraglitch::gui
{

PowerButton::PowerButton(const juce::String& /*buttonName*/)
{
    // LookAndFeel expected to be set globally in PluginEditor

    // No title text
    setButtonText(juce::String());
    setClickingTogglesState(true);

    configureButtonStyle();
}

void PowerButton::paint(juce::Graphics& g)
{
    // Let LookAndFeel handle drawing
    juce::ToggleButton::paint(g);
}

void PowerButton::resized()
{
    juce::ToggleButton::resized();
}

void PowerButton::configureButtonStyle()
{
    setToggleState(false, juce::dontSendNotification); // Default off
    setOpaque(false);
}

} // namespace ultraglitch::gui