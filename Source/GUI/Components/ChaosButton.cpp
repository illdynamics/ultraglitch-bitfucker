#include "ChaosButton.h"
#include "../LookAndFeel/GlitchLookAndFeel.h"

namespace ultraglitch::gui
{

ChaosButton::ChaosButton(const juce::String& name)
    : juce::TextButton (name)
{
    setClickingTogglesState(true);
    juce::TextButton::setToggleState(false, juce::dontSendNotification);
}

void ChaosButton::paint(juce::Graphics& g)
{
    auto& lf = getLookAndFeel();

    juce::Colour backgroundColour = juce::TextButton::getToggleState()
        ? lf.findColour(juce::TextButton::buttonOnColourId)
        : lf.findColour(juce::TextButton::buttonColourId);
    
    juce::Colour textColour = juce::TextButton::getToggleState()
        ? lf.findColour(juce::TextButton::textColourOnId)
        : lf.findColour(juce::TextButton::textColourOffId);

    lf.drawButtonBackground(g, *this, backgroundColour, isMouseOverOrDragging(), isDown());

    g.setColour(textColour);
    g.setFont(lf.getTextButtonFont(*this, getHeight()));
    g.drawFittedText(getButtonText(), getLocalBounds(), juce::Justification::centred, 1);
}

void ChaosButton::resized()
{
}

} // namespace ultraglitch::gui
