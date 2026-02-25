#include "PowerButton.h"
#include "../LookAndFeel/GlitchLookAndFeel.h"

namespace ultraglitch::gui
{

PowerButton::PowerButton(const juce::String& buttonName)
{
    juce::ignoreUnused(buttonName);

    setButtonText({});
    setClickingTogglesState(true);

    configureButtonStyle();
}

void PowerButton::paint(juce::Graphics& g)
{
    juce::ToggleButton::paint(g);
}

void PowerButton::resized()
{
    juce::ToggleButton::resized();
}

void PowerButton::configureButtonStyle()
{
    setToggleState(false, juce::dontSendNotification);
    setOpaque(false);
}

} // namespace ultraglitch::gui