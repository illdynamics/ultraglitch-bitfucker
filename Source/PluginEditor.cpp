#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "GUI/MainPanel.h"
#include "Common/ParameterIDs.h"

//==============================================================================
UltraGlitchAudioProcessorEditor::UltraGlitchAudioProcessorEditor (UltraGlitchAudioProcessor& p, PluginParameters& params)
    : AudioProcessorEditor (&p), processor_ (p), parameters_ (params)
{
    // Set our owned GlitchLookAndFeel as the global default
    juce::LookAndFeel::setDefaultLookAndFeel(&glitchLookAndFeel_);

    // Create main panel, passing PluginParameters
    main_panel_ = std::make_unique<ultraglitch::gui::MainPanel>(parameters_);
    addAndMakeVisible (*main_panel_); // Use dereferenced unique_ptr

    // Set up global parameter attachments managed by PluginEditor
    setup_parameter_attachments();

    // Set size based on plugin config, or a default
    setSize (800, 600); // Or use config::EDITOR_WIDTH, config::EDITOR_HEIGHT
}

UltraGlitchAudioProcessorEditor::~UltraGlitchAudioProcessorEditor()
{
    // Clean up LookAndFeel if it was set globally and owned by editor
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

//==============================================================================
void UltraGlitchAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void UltraGlitchAudioProcessorEditor::resized()
{
    if (main_panel_)
    {
        main_panel_->setBounds (getLocalBounds());
    }
}

void UltraGlitchAudioProcessorEditor::setup_parameter_attachments()
{
    auto& apvts = parameters_.get_value_tree_state(); // Get APVTS from PluginParameters

    // Attach global gain knob from MainPanel
    if (auto* globalGainKnob = main_panel_->getGlobalGainKnob())
    {
        global_gain_attachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, ultraglitch::params::Global_Gain, *globalGainKnob
        );
    }

    // Attach chaos button from MainPanel
    if (auto* chaosButton = main_panel_->getChaosButton())
    {
        chaos_button_attachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, ultraglitch::params::Global_ChaosMode, *chaosButton
        );
    }

    // Effect Modules handle their own parameter attachments internally.
    // So, no need to iterate through effect modules here.
}