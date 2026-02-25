#include "ChaosButton.h"
#include "../LookAndFeel/GlitchLookAndFeel.h"

namespace ultraglitch::gui
{

ChaosButton::ChaosButton(const juce::String& /*name*/)
    : juce::TextButton(juce::String()) // explicit: avoids {} ambiguity
{
    setClickingTogglesState(true);
    juce::TextButton::setToggleState(false, juce::dontSendNotification);

    // No text ever
    setButtonText(juce::String());

    // Make sure NOTHING draws a background via standard colour IDs
    setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    setColour(juce::TextButton::textColourOffId, juce::Colours::transparentBlack);
    setColour(juce::TextButton::textColourOnId, juce::Colours::transparentBlack);

    // No opaque repaint optimisation that could block parent
    setOpaque(false);

    // Still clickable even though invisible
    setInterceptsMouseClicks(true, false);
}

void ChaosButton::paint(juce::Graphics& /*g*/)
{
    // INTENTIONALLY DRAW NOTHING.
    // This makes the long CHAOS button disappear visually,
    // but it still receives clicks and toggles state/attachments.
}

void ChaosButton::resized()
{
}

} // namespace ultraglitch::gui