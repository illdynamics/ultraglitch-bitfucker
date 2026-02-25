#include "GlitchKnob.h"
#include "../LookAndFeel/GlitchLookAndFeel.h" // Needed for color access if not set globally

namespace ultraglitch::gui
{

GlitchKnob::GlitchKnob(const juce::String& knobName)
{
    // The LookAndFeel is expected to be set globally in PluginEditor
    // For individual overrides, you might still use setLookAndFeel here.
    
    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    setRange(0.0, 1.0, 0.01);
    setValue(0.5);
    
    configureKnobStyle();
    setupLabel(knobName);
}

void GlitchKnob::paint(juce::Graphics& g)
{
    // Painting is handled by the LookAndFeel
    juce::Slider::paint(g);
}

void GlitchKnob::resized()
{
    juce::Slider::resized();
    
    const int labelHeight = 20;
    nameLabel_.setBounds(0, getHeight() - labelHeight, getWidth(), labelHeight);
}

void GlitchKnob::configureKnobStyle()
{
    // Set custom colours from GlitchLookAndFeel
    // These colours will be picked up by the global GlitchLookAndFeel drawing methods.
    // However, if specific to this knob, they can be set here.
    setColour(juce::Slider::rotarySliderFillColourId, findColour(juce::Slider::rotarySliderFillColourId));
    setColour(juce::Slider::thumbColourId, findColour(juce::Slider::thumbColourId));
    setColour(juce::Slider::textBoxTextColourId, findColour(juce::Slider::textBoxTextColourId));
    setColour(juce::Slider::textBoxBackgroundColourId, findColour(juce::Slider::textBoxBackgroundColourId));
    setColour(juce::Slider::textBoxOutlineColourId, findColour(juce::Slider::textBoxOutlineColourId));

    // Make textbox transparent for aesthetics
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void GlitchKnob::setupLabel(const juce::String& knobName)
{
    nameLabel_.setText(knobName, juce::dontSendNotification);
    nameLabel_.setJustificationType(juce::Justification::centred);
    nameLabel_.setFont(juce::Font(juce::FontOptions(12.0f)).boldened());
    nameLabel_.setColour(juce::Label::textColourId, findColour(juce::Label::textColourId));
    addAndMakeVisible(nameLabel_);
}

} // namespace ultraglitch::gui