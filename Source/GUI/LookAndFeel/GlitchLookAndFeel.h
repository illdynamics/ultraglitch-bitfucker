#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

namespace ultraglitch::gui
{
class GlitchLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GlitchLookAndFeel();
    ~GlitchLookAndFeel() override = default;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu, const juce::String& text,
                           const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    void drawBubble(juce::Graphics& g, juce::BubbleComponent& bubble,
                    const juce::Point<float>& tip, const juce::Rectangle<float>& body) override;

    void drawProgressBar(juce::Graphics& g, juce::ProgressBar& progressBar,
                         int width, int height, double progress,
                         const juce::String& textToShow) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    juce::Font getLabelFont(juce::Label& label) override;
    juce::Font getComboBoxFont(juce::ComboBox& box) override;

private:
    juce::Colour getPrimaryBackgroundColour() const; // Renamed
    juce::Colour getSecondaryBackgroundColour() const; // Renamed
    juce::Colour getAccentColour() const; // Renamed
    juce::Colour getHighlightColour() const; // Renamed
    juce::Colour getTextColour() const; // Renamed
    juce::Colour getDisabledColour() const; // Renamed

    void drawRoundedRectWithGlow(juce::Graphics& g, const juce::Rectangle<float>& bounds, // Renamed
                                     float cornerSize, const juce::Colour& fillColour,
                                     const juce::Colour& glowColour, float glowIntensity = 1.0f);

    void drawGlitchPattern(juce::Graphics& g, const juce::Rectangle<float>& area, // Renamed
                             const juce::Colour& patternColour, float intensity = 0.1f);

    juce::Path createRoundedRectPath(const juce::Rectangle<float>& bounds, float cornerSize) const; // Renamed
    juce::Path createRotarySliderPath(float radius, float centreX, float centreY, // Renamed
                                         float rotaryStartAngle, float rotaryEndAngle) const;

    juce::DropShadow createGlowShadow(const juce::Colour& glowColour, float intensity = 1.0f) const; // Renamed
};
} // namespace ultraglitch::gui