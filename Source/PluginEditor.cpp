#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "GUI/MainPanel.h"
#include "Common/ParameterIDs.h"
#include "BinaryData.h"

//==============================================================================
UltraGlitchAudioProcessorEditor::UltraGlitchAudioProcessorEditor (UltraGlitchAudioProcessor& p, PluginParameters& params)
    : AudioProcessorEditor (&p), processor_ (p), parameters_ (params)
{
    // Set our owned GlitchLookAndFeel as the global default
    juce::LookAndFeel::setDefaultLookAndFeel(&glitchLookAndFeel_);

    // Create main panel, passing PluginParameters
    main_panel_ = std::make_unique<ultraglitch::gui::MainPanel>(parameters_);
    addAndMakeVisible (*main_panel_);

    // Set up global parameter attachments managed by PluginEditor
    setup_parameter_attachments();

    setSize (800, 600);

    // We paint the full background ourselves.
    setOpaque (true);
}

UltraGlitchAudioProcessorEditor::~UltraGlitchAudioProcessorEditor()
{
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

//==============================================================================
void UltraGlitchAudioProcessorEditor::paint (juce::Graphics& g)
{
    static const juce::Image bg = juce::ImageCache::getFromMemory (
        BinaryData::ugbf1_png,
        BinaryData::ugbf1_pngSize);

    if (bg.isValid())
    {
        constexpr int cropTopPx    = 44;  // ← remove big green title
        constexpr int cropBottomPx = 40;  // ← remove bottom baked text

        auto src = bg.getBounds().toFloat();

        if (src.getHeight() > (float)(cropTopPx + cropBottomPx + 1))
        {
            src.setY(src.getY() + cropTopPx);
            src.setHeight(src.getHeight() - cropTopPx - cropBottomPx);
        }

        auto dst = getLocalBounds().toFloat();

        g.drawImage(bg,
                    dst.getX(), dst.getY(), dst.getWidth(), dst.getHeight(),
                    src.getX(), src.getY(), src.getWidth(), src.getHeight());
    }
    else
    {
        g.fillAll(juce::Colours::black);
    }
}

void UltraGlitchAudioProcessorEditor::resized()
{
    if (main_panel_)
        main_panel_->setBounds (getLocalBounds());
}

void UltraGlitchAudioProcessorEditor::setup_parameter_attachments()
{
    auto& apvts = parameters_.get_value_tree_state();

    if (auto* globalGainKnob = main_panel_->getGlobalGainKnob())
    {
        global_gain_attachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, ultraglitch::params::Global_Gain, *globalGainKnob
        );
    }

    if (auto* chaosButton = main_panel_->getChaosButton())
    {
        chaos_button_attachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, ultraglitch::params::Global_ChaosMode, *chaosButton
        );
    }
}
