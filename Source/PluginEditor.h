#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "GUI/MainPanel.h"
#include "GUI/LookAndFeel/GlitchLookAndFeel.h"
#include "Parameters/PluginParameters.h"

// Forward declarations
class UltraGlitchAudioProcessor;

class UltraGlitchAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit UltraGlitchAudioProcessorEditor (UltraGlitchAudioProcessor& p, PluginParameters& params);
    ~UltraGlitchAudioProcessorEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    UltraGlitchAudioProcessor& processor_;
    PluginParameters& parameters_;

    // Owned LookAndFeel instance — must be declared before any GUI components
    ultraglitch::gui::GlitchLookAndFeel glitchLookAndFeel_;

    std::unique_ptr<ultraglitch::gui::MainPanel> main_panel_;

    // Private helper methods
    void setup_parameter_attachments();

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> global_gain_attachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> chaos_button_attachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UltraGlitchAudioProcessorEditor)
};

#endif // PLUGINEDITOR_H_INCLUDED
