#include "GlitchKnob.h"
#include "../LookAndFeel/GlitchLookAndFeel.h"

namespace ultraglitch::gui
{

// Shared LookAndFeel instance for knobs only.
// This guarantees the rotary knob drawing comes back even if global LAF isn't set.
static GlitchLookAndFeel& getSharedKnobLookAndFeel()
{
    static GlitchLookAndFeel laf;
    return laf;
}

GlitchKnob::GlitchKnob(const juce::String& /*knobName*/)
{
    // Force our LookAndFeel for the knob visuals
    setLookAndFeel(&getSharedKnobLookAndFeel());

    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);

    // Keep UI clean: no textbox/fields
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    setRange(0.0, 1.0, 0.01);
    setValue(0.5);

    configureKnobStyle();

    // transparency-friendly
    setOpaque(false);
}

void GlitchKnob::paint(juce::Graphics& g)
{
    // This will now reliably call GlitchLookAndFeel::drawRotarySlider
    juce::Slider::paint(g);
}

void GlitchKnob::resized()
{
    juce::Slider::resized();
}

void GlitchKnob::configureKnobStyle()
{
    // No per-component colour overrides.
    // Let GlitchLookAndFeel control the look.
}

} // namespace ultraglitch::gui