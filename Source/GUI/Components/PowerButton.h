#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../LookAndFeel/GlitchLookAndFeel.h" // Use our custom LookAndFeel

namespace ultraglitch::gui
{
class PowerButton : public juce::ToggleButton
{
public:
    explicit PowerButton(const juce::String& buttonName); // Removed parameterID, as it's for attachment in Editor
    ~PowerButton() override = default;

    void paint(juce::Graphics& g) override; // Override needed for custom power symbol or LED
    void resized() override;

private:
    void configureButtonStyle(); // Renamed
    // GlitchLookAndFeel::drawToggleButton will handle the drawing
    // If a custom power symbol is needed, paint() override will draw it.
};
} // namespace ultraglitch::gui