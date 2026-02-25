#include "GlitchLookAndFeel.h"
#include "../../Common/PluginConfig.h" // For EDITOR_WIDTH etc if needed

namespace ultraglitch::gui
{

GlitchLookAndFeel::GlitchLookAndFeel()
{
    // Basic color scheme from tasq.md
    setColour(juce::ResizableWindow::backgroundColourId, getPrimaryBackgroundColour());
    setColour(juce::TextButton::buttonColourId, getSecondaryBackgroundColour());
    setColour(juce::TextButton::buttonOnColourId, getAccentColour());
    setColour(juce::Slider::thumbColourId, getHighlightColour());
    setColour(juce::Slider::rotarySliderFillColourId, getAccentColour().withAlpha(0.7f));
    setColour(juce::Slider::rotarySliderOutlineColourId, getSecondaryBackgroundColour());
    setColour(juce::Slider::textBoxTextColourId, getTextColour());
    setColour(juce::Slider::textBoxBackgroundColourId, getSecondaryBackgroundColour().brighter(0.1f));
    setColour(juce::Slider::textBoxOutlineColourId, getAccentColour().darker(0.2f));
    setColour(juce::Label::textColourId, getTextColour());
    setColour(juce::ComboBox::backgroundColourId, getSecondaryBackgroundColour());
    setColour(juce::ComboBox::arrowColourId, getTextColour());
    setColour(juce::ComboBox::outlineColourId, getAccentColour().darker(0.2f));
    setColour(juce::PopupMenu::backgroundColourId, getSecondaryBackgroundColour());
    setColour(juce::PopupMenu::textColourId, getTextColour());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, getHighlightColour());
    setColour(juce::PopupMenu::highlightedTextColourId, getPrimaryBackgroundColour());

    // Set default font to clean sans-serif
    setDefaultSansSerifTypefaceName("Arial");

    // Additional colour overrides (these were in findColour override, moved here for correctness)
    setColour(juce::ToggleButton::textColourId, getTextColour());
    setColour(juce::TextButton::textColourOffId, getTextColour());
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::backgroundColourId, getPrimaryBackgroundColour().darker(0.1f));
}

// Helper methods for colors (matching tasq.md requirements)
juce::Colour GlitchLookAndFeel::getPrimaryBackgroundColour() const { return juce::Colour::fromString("#FF1a1a1a"); } // Dark background
juce::Colour GlitchLookAndFeel::getSecondaryBackgroundColour() const { return juce::Colour::fromString("#FF2a2a2a"); }
juce::Colour GlitchLookAndFeel::getAccentColour() const { return juce::Colour::fromString("#FF00FFFF"); } // Cyan
juce::Colour GlitchLookAndFeel::getHighlightColour() const { return juce::Colour::fromString("#FFFF00FF"); } // Magenta
juce::Colour GlitchLookAndFeel::getTextColour() const { return juce::Colour::fromString("#FF00FF00").brighter(0.5f); } // Green
juce::Colour GlitchLookAndFeel::getDisabledColour() const { return juce::Colour::fromString("#FF505050"); }

void GlitchLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPosProportional, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& slider)
{
    auto outline = slider.findColour(juce::Slider::rotarySliderOutlineColourId);
    auto fill    = slider.findColour(juce::Slider::rotarySliderFillColourId);

    auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(1.0f);
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    float toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Draw track
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(outline);
    g.strokePath(backgroundArc, juce::PathStrokeType(slider.getTextBoxHeight() * 0.15f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));

    // Draw filled track based on value
    if (slider.isEnabled())
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(fill);
        g.strokePath(valueArc, juce::PathStrokeType(slider.getTextBoxHeight() * 0.15f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
    }

    // Draw thumb
    juce::Path p;
    auto thumbWidth = slider.getTextBoxHeight() * 0.25f;
    p.addEllipse(-thumbWidth / 2, -thumbWidth / 2, thumbWidth, thumbWidth);
    
    g.setColour(slider.findColour(juce::Slider::thumbColourId)); // Use accent color for thumb
    g.fillPath(p, juce::AffineTransform::rotation(toAngle).translated(centreX, centreY));
}

void GlitchLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float minSliderPos, float maxSliderPos,
                                        const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    juce::ignoreUnused(minSliderPos, maxSliderPos);
    g.fillAll(slider.findColour(juce::Slider::backgroundColourId)); // Fill slider background
    
    // Draw track
    juce::Rectangle<float> track;
    if (style == juce::Slider::LinearBar || style == juce::Slider::LinearBarVertical)
        track = slider.isHorizontal() ? juce::Rectangle<float>(x, y + height * 0.5f - 2.0f, width, 4.0f)
                                      : juce::Rectangle<float>(x + width * 0.5f - 2.0f, y, 4.0f, height);
    else
        track = slider.isHorizontal() ? juce::Rectangle<float>(x, y + height * 0.5f - 2.0f, width, 4.0f)
                                      : juce::Rectangle<float>(x + width * 0.5f - 2.0f, y, 4.0f, height);

    g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId)); // Use track colour
    g.fillRect(track);

    // Draw fill based on value
    juce::Rectangle<float> fillArea;
    if (slider.isHorizontal())
        fillArea = juce::Rectangle<float>(track.getX(), track.getY(), sliderPos - track.getX(), track.getHeight());
    else
        fillArea = juce::Rectangle<float>(track.getX(), sliderPos, track.getWidth(), track.getBottom() - sliderPos);
    
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId)); // Use fill colour
    g.fillRect(fillArea);

    // Draw thumb
    juce::Rectangle<float> thumb;
    if (slider.isHorizontal())
        thumb = juce::Rectangle<float>(sliderPos - 5.0f, track.getY() - 3.0f, 10.0f, track.getHeight() + 6.0f);
    else
        thumb = juce::Rectangle<float>(track.getX() - 3.0f, sliderPos - 5.0f, track.getWidth() + 6.0f, 10.0f);
    
    g.setColour(slider.findColour(juce::Slider::thumbColourId));
    g.fillRoundedRectangle(thumb, 2.0f);
}

void GlitchLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                            const juce::Colour& backgroundColour,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    auto buttonArea = button.getLocalBounds().toFloat();
    auto baseColour = backgroundColour;

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(shouldDrawButtonAsDown ? 0.2f : 0.1f);
    
    g.setColour(baseColour);
    g.fillRoundedRectangle(buttonArea, 4.0f); // Rounded corners

    g.setColour(getAccentColour().darker(0.2f));
    g.drawRoundedRectangle(buttonArea, 4.0f, 1.0f); // Border
}

void GlitchLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                        bool shouldDrawButtonAsHighlighted,
                                        bool shouldDrawButtonAsDown)
{
    auto buttonArea = button.getLocalBounds().toFloat().reduced(2.0f);
    auto cornerSize = 4.0f;
    
    juce::Colour fillColour = button.getToggleState() ? getAccentColour() : getSecondaryBackgroundColour();
    juce::Colour outlineColour = button.getToggleState() ? getAccentColour().brighter(0.5f) : getDisabledColour();

    if (shouldDrawButtonAsHighlighted)
        fillColour = fillColour.brighter(0.1f);

    if (shouldDrawButtonAsDown)
        fillColour = fillColour.brighter(0.2f);

    g.setColour(fillColour);
    g.fillRoundedRectangle(buttonArea, cornerSize);
    
    g.setColour(outlineColour);
    g.drawRoundedRectangle(buttonArea, cornerSize, 1.5f);

    // Draw a "glow" effect if toggled on
    if (button.getToggleState())
    {
        drawRoundedRectWithGlow(g, buttonArea, cornerSize, fillColour, getAccentColour(), 0.5f);
    }
}

void GlitchLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                    int buttonX, int buttonY, int buttonW, int buttonH,
                                    juce::ComboBox& box)
{
    juce::ignoreUnused(width, height, isButtonDown, buttonY, buttonH);
    auto boxBounds = box.getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(boxBounds, 4.0f);

    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(boxBounds, 4.0f, 1.0f);

    // Draw the arrow
    juce::Path p;
    p.addTriangle(buttonX + buttonW * 0.2f, (float)height * 0.4f,
                  buttonX + buttonW * 0.8f, (float)height * 0.4f,
                  buttonX + buttonW * 0.5f, (float)height * 0.6f);
    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    g.fillPath(p);
}

void GlitchLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                        bool isSeparator, bool isActive, bool isHighlighted,
                                        bool isTicked, bool hasSubMenu, const juce::String& text,
                                        const juce::String& shortcutKeyText,
                                        const juce::Drawable* icon, const juce::Colour* textColour)
{
    juce::ignoreUnused(shortcutKeyText, icon);
    if (isSeparator)
    {
        auto r = area.reduced(5, 0);
        g.setColour(getSecondaryBackgroundColour());
        g.fillRect(r.withHeight(1));
        return;
    }

    auto textRect = area.reduced(2);
    if (isHighlighted)
    {
        g.setColour(getHighlightColour());
        g.fillRoundedRectangle(textRect.toFloat(), 2.0f);
    }

    g.setColour(isActive ? (textColour != nullptr ? *textColour : findColour(juce::PopupMenu::textColourId))
                       : findColour(juce::PopupMenu::textColourId).withAlpha(0.6f));

    g.setFont(juce::Font(juce::FontOptions(14.0f)));
    g.drawFittedText(text, textRect.reduced(3, 0), juce::Justification::centredLeft, 1);

    if (isTicked)
    {
        juce::Path tick;
        tick.addTriangle(textRect.getX() + 5.0f, textRect.getCentreY(),
                         textRect.getX() + 10.0f, textRect.getCentreY() + 5.0f,
                         textRect.getX() + 15.0f, textRect.getCentreY() - 5.0f);
        g.fillPath(tick);
    }

    if (hasSubMenu)
    {
        juce::Path arrow;
        arrow.addTriangle(textRect.getRight() - 15.0f, textRect.getCentreY() - 5.0f,
                          textRect.getRight() - 10.0f, textRect.getCentreY(),
                          textRect.getRight() - 15.0f, textRect.getCentreY() + 5.0f);
        g.fillPath(arrow);
    }
}

void GlitchLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    auto background = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height);
    g.setColour(getSecondaryBackgroundColour());
    g.fillRoundedRectangle(background, 4.0f);

    g.setColour(getAccentColour().darker(0.2f));
    g.drawRoundedRectangle(background, 4.0f, 1.0f);
}

void GlitchLookAndFeel::drawBubble(juce::Graphics& g, juce::BubbleComponent& bubble,
                                  const juce::Point<float>& tip, const juce::Rectangle<float>& body)
{
    // Default JUCE bubble drawing with custom colours
    juce::LookAndFeel_V4::drawBubble(g, bubble, tip, body);
}

void GlitchLookAndFeel::drawProgressBar(juce::Graphics& g, juce::ProgressBar& progressBar,
                                       int width, int height, double progress,
                                       const juce::String& textToShow)
{
    juce::ignoreUnused(progressBar);
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height).reduced(1.0f);
    g.setColour(getSecondaryBackgroundColour());
    g.fillRoundedRectangle(bounds, 4.0f);

    juce::Rectangle<float> progressBounds = bounds.withWidth(bounds.getWidth() * (float)progress);
    g.setColour(getHighlightColour());
    g.fillRoundedRectangle(progressBounds, 4.0f);

    g.setColour(getAccentColour().darker(0.2f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    if (textToShow.isNotEmpty())
    {
        g.setColour(getTextColour());
        g.setFont(juce::Font(juce::FontOptions(14.0f)));
        g.drawFittedText(textToShow, bounds.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Font GlitchLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return juce::Font(juce::FontOptions(juce::jmin(15.0f, (float)buttonHeight * 0.6f)));
}

juce::Font GlitchLookAndFeel::getLabelFont(juce::Label& label)
{
    return juce::Font(juce::FontOptions(juce::jmin(15.0f, (float)label.getHeight() * 0.6f)));
}

juce::Font GlitchLookAndFeel::getComboBoxFont(juce::ComboBox& box)
{
    return juce::Font(juce::FontOptions(juce::jmin(15.0f, (float)box.getHeight() * 0.6f)));
}

void GlitchLookAndFeel::drawRoundedRectWithGlow(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                             float cornerSize, const juce::Colour& fillColour,
                                             const juce::Colour& glowColour, float glowIntensity)
{
    // Draw a slightly larger, blurred version of the shape for the glow effect
    auto glowBounds = bounds.expanded(glowIntensity * 5.0f);
    g.setColour(glowColour.withAlpha(glowIntensity));
    g.fillRoundedRectangle(glowBounds, cornerSize + glowIntensity * 2.0f); // Larger rounded corners for blur effect
    
    // Draw the actual shape on top
    g.setColour(fillColour);
    g.fillRoundedRectangle(bounds, cornerSize);
}

void GlitchLookAndFeel::drawGlitchPattern(juce::Graphics& g, const juce::Rectangle<float>& area,
                                         const juce::Colour& patternColour, float intensity)
{
    // Simple glitch pattern: draw random lines or rectangles
    juce::Random rng;
    g.setColour(patternColour.withAlpha(intensity * 0.5f));
    
    for (int i = 0; i < 5 + (int)(intensity * 20); ++i)
    {
        float x1 = area.getX() + rng.nextFloat() * area.getWidth();
        float y1 = area.getY() + rng.nextFloat() * area.getHeight();
        float x2 = x1 + rng.nextFloat() * 10.0f;
        float y2 = y1 + rng.nextFloat() * 2.0f;
        g.drawLine(x1, y1, x2, y2, 1.0f);
    }
}

juce::Path GlitchLookAndFeel::createRoundedRectPath(const juce::Rectangle<float>& bounds, float cornerSize) const
{
    juce::Path p;
    p.addRoundedRectangle(bounds, cornerSize);
    return p;
}

juce::Path GlitchLookAndFeel::createRotarySliderPath(float radius, float centreX, float centreY,
                                                     float rotaryStartAngle, float rotaryEndAngle) const
{
    juce::Path p;
    p.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    return p;
}

juce::DropShadow GlitchLookAndFeel::createGlowShadow(const juce::Colour& glowColour, float intensity) const
{
    return juce::DropShadow(glowColour.withAlpha(intensity), 10, {0, 0});
}

} // namespace ultraglitch::gui