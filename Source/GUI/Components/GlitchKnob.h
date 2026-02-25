#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace ultraglitch::gui
{
class GlitchKnob : public juce::Slider
{
public:
    explicit GlitchKnob(const juce::String& knobName);
    ~GlitchKnob() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void configureKnobStyle();
};
} // namespace ultraglitch::gui