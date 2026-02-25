#ifndef PLUGINPARAMETERS_H
#define PLUGINPARAMETERS_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <vector>
#include <memory>

// Forward declarations
class ParameterListener;

enum class ParameterType
{
    Float,
    Bool,
    Choice
};

struct ParameterDefinition
{
    juce::String id;
    juce::String name;
    juce::String label;
    ParameterType type;
    float minValue;
    float maxValue;
    float stepValue;
    float skewFactor;
    float defaultValue;
    std::vector<juce::String> choices;
};

class PluginParameters
{
public:
    explicit PluginParameters(juce::AudioProcessor& processor);
    ~PluginParameters();
    
    // Delete copy constructor and assignment operator
    PluginParameters(const PluginParameters&) = delete;
    PluginParameters& operator=(const PluginParameters&) = delete;
    
    // Static method to get all parameter definitions — returns const ref to avoid per-call allocation
    static const std::vector<ParameterDefinition>& get_parameter_definitions();
    
    // Parameter value access
    float get_parameter_value(const juce::String& parameter_id) const;
    void set_parameter_value(const juce::String& parameter_id, float new_value);
    juce::String get_parameter_text(const juce::String& parameter_id) const;
    
    // Direct parameter access
    juce::RangedAudioParameter* get_parameter(const juce::String& parameter_id) const;
    
    // Value tree state access
    juce::AudioProcessorValueTreeState& get_value_tree_state();
    const juce::AudioProcessorValueTreeState& get_value_tree_state() const;
    
    // Listener management
    void add_listener(juce::AudioProcessorValueTreeState::Listener* listener);
    void remove_listener(juce::AudioProcessorValueTreeState::Listener* listener);
    
    // Custom listener management
    void add_parameter_listener(ParameterListener* listener);
    void remove_parameter_listener(ParameterListener* listener);
    
    // State persistence
    bool save_state_to_xml(juce::XmlElement& xml);
    bool load_state_from_xml(const juce::XmlElement& xml);
    
    // Reset to defaults
    void reset_to_defaults();
    
private:
    juce::AudioProcessorValueTreeState apvts_;
    std::vector<std::unique_ptr<juce::ParameterAttachment>> parameter_attachments_;
    juce::ListenerList<ParameterListener> parameter_listeners_;
    
    // Create parameter layout
    juce::AudioProcessorValueTreeState::ParameterLayout create_parameter_layout();
    
    // Initialize parameter attachments
    void initialize_parameter_attachments();
    
    // Handle parameter changes
    void handle_parameter_change(const juce::String& parameter_id, float new_value);
};

class ParameterListener
{
public:
    virtual ~ParameterListener() = default;
    
    virtual void parameter_changed(const juce::String& parameter_id, float new_value) = 0;
};

#endif // PLUGINPARAMETERS_H