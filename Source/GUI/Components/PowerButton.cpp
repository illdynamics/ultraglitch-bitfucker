#include "PowerButton.h"
#include "../LookAndFeel/GlitchLookAndFeel.h" // Needed for color access if not set globally

namespace ultraglitch::gui
{

PowerButton::PowerButton(const juce::String& buttonName)
{
    // The LookAndFeel is expected to be set globally in PluginEditor
    
    setButtonText(buttonName);
    setClickingTogglesState(true);
    
    configureButtonStyle();
}

void PowerButton::paint(juce::Graphics& g)
{
    // Painting is handled by the LookAndFeel's drawToggleButton
    juce::ToggleButton::paint(g);

    // If a custom power symbol is needed *inside* the button, it can be drawn here.
    // For now, GlitchLookAndFeel handles the LED-style glow based on toggle state.
}

void PowerButton::resized()
{
    juce::ToggleButton::resized();
}

void PowerButton::configureButtonStyle()
{
    // Set custom colours from GlitchLookAndFeel
    setColour(juce::ToggleButton::textColourId, findColour(juce::ToggleButton::textColourId));
    setColour(juce::ToggleButton::tickColourId, findColour(juce::ToggleButton::tickColourId));
    setColour(juce::ToggleButton::tickDisabledColourId, findColour(juce::ToggleButton::tickDisabledColourId));

    // For power buttons, the default tick is often not desired.
    // We expect GlitchLookAndFeel to draw a custom 'on' state.
    setToggleState(false, juce::dontSendNotification); // Default off
}

} // namespace ultraglitch::gui