#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../LookAndFeel/GlitchLookAndFeel.h" // Use our custom LookAndFeel

namespace ultraglitch::gui
{
class GlitchKnob : public juce::Slider
{
public:
    explicit GlitchKnob(const juce::String& knobName); // Removed parameterID, as it's for attachment in Editor
    ~GlitchKnob() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label nameLabel_; // Renamed member
    // No explicit LookAndFeel member; GlitchLookAndFeel will be set globally or per component via setLookAndFeel
    
    void configureKnobStyle(); // Renamed
    void setupLabel(const juce::String& knobName); // Renamed
};

// No separate GlitchKnobLookAndFeel is needed if GlitchLookAndFeel handles all rotary slider drawing.
// If a specific knob needs a *different* custom drawing, then this nested LookAndFeel might be useful,
// but for a general "GlitchKnob", the global GlitchLookAndFeel should suffice.
// I will remove GlitchKnobLookAndFeel and let GlitchLookAndFeel handle all knob drawing as per tasq.md.
// The tasq.md: "Custom rotary knob drawing" in GlitchLookAndFeel.
} // namespace ultraglitch::gui