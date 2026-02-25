#include "PluginParameters.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include "../Common/ParameterIDs.h"

PluginParameters::PluginParameters(juce::AudioProcessor& processor)
    : apvts_(processor, nullptr, "PARAMETERS", create_parameter_layout())
{
    initialize_parameter_attachments();
}

PluginParameters::~PluginParameters()
{
    // Clean up attachments
    for (auto& attachment : parameter_attachments_)
    {
        attachment.reset();
    }
    parameter_attachments_.clear();
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginParameters::create_parameter_layout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Add all parameters from the definitions
    for (const auto& param_def : get_parameter_definitions())
    {
        switch (param_def.type)
        {
            case ParameterType::Float:
            {
                auto range = juce::NormalisableRange<float>(
                    param_def.minValue,
                    param_def.maxValue,
                    param_def.stepValue,
                    param_def.skewFactor
                );
                
                layout.add(std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID(param_def.id, 1),
                    param_def.name,
                    range,
                    param_def.defaultValue,
                    juce::AudioParameterFloatAttributes()
                        .withLabel(param_def.label)
                ));
                break;
            }
            
            case ParameterType::Bool:
            {
                layout.add(std::make_unique<juce::AudioParameterBool>(
                    juce::ParameterID(param_def.id, 1),
                    param_def.name,
                    static_cast<bool>(param_def.defaultValue),
                    juce::AudioParameterBoolAttributes()
                        .withLabel(param_def.label)
                ));
                break;
            }
            
            case ParameterType::Choice:
            {
                juce::StringArray choices;
                for (const auto& choice : param_def.choices)
                {
                    choices.add(choice);
                }
                
                layout.add(std::make_unique<juce::AudioParameterChoice>(
                    juce::ParameterID(param_def.id, 1),
                    param_def.name,
                    choices,
                    static_cast<int>(param_def.defaultValue),
                    juce::AudioParameterChoiceAttributes()
                        .withLabel(param_def.label)
                ));
                break;
            }
        }
    }
    
    return layout;
}

void PluginParameters::initialize_parameter_attachments()
{
    // Create attachments for all parameters
    for (const auto& param_def : get_parameter_definitions())
    {
        auto* parameter = apvts_.getParameter(param_def.id);
        if (parameter != nullptr)
        {
            // Create a lambda-based attachment for this parameter
            auto attachment = std::make_unique<juce::ParameterAttachment>(
                *parameter,
                [this, param_id = param_def.id](float normalizedValue) // normalized_value from APVTS
                {
                    handle_parameter_change(param_id, normalizedValue);
                },
                nullptr
            );
            
            parameter_attachments_.push_back(std::move(attachment));
        }
    }
}

float PluginParameters::get_parameter_value(const juce::String& parameter_id) const
{
    if (auto* rawValue = apvts_.getRawParameterValue(parameter_id))
    {
        // getRawParameterValue returns the actual (denormalized) parameter value
        return rawValue->load();
    }
    return 0.0f;
}

void PluginParameters::set_parameter_value(const juce::String& parameter_id, float new_real_value)
{
    if (auto* parameter = apvts_.getParameter(parameter_id))
    {
        // Convert real value to normalized 0-1 before setting
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(parameter))
        {
            rp->setValueNotifyingHost(rp->convertTo0to1(new_real_value));
        }
    }
}

juce::String PluginParameters::get_parameter_text(const juce::String& parameter_id) const
{
    auto* parameter = apvts_.getParameter(parameter_id);
    if (parameter != nullptr)
    {
        // getText expects normalized 0-1 value
        return parameter->getText(parameter->getValue(), 0);
    }
    return juce::String();
}

juce::RangedAudioParameter* PluginParameters::get_parameter(const juce::String& parameter_id) const
{
    return apvts_.getParameter(parameter_id);
}

juce::AudioProcessorValueTreeState& PluginParameters::get_value_tree_state()
{
    return apvts_;
}

const juce::AudioProcessorValueTreeState& PluginParameters::get_value_tree_state() const
{
    return apvts_;
}

void PluginParameters::add_listener(juce::AudioProcessorValueTreeState::Listener* listener)
{
    // Register for all defined parameters
    for (const auto& param_def : get_parameter_definitions())
    {
        apvts_.addParameterListener(param_def.id, listener);
    }
}

void PluginParameters::remove_listener(juce::AudioProcessorValueTreeState::Listener* listener)
{
    // Unregister from all defined parameters
    for (const auto& param_def : get_parameter_definitions())
    {
        apvts_.removeParameterListener(param_def.id, listener);
    }
}

void PluginParameters::handle_parameter_change(const juce::String& parameter_id, float new_value)
{
    // Note: ParameterAttachment callback provides the denormalized (real) value
    parameter_listeners_.call([&](ParameterListener& listener)
    {
        listener.parameter_changed(parameter_id, new_value);
    });
}

void PluginParameters::add_parameter_listener(ParameterListener* listener)
{
    parameter_listeners_.add(listener);
}

void PluginParameters::remove_parameter_listener(ParameterListener* listener)
{
    parameter_listeners_.remove(listener);
}

bool PluginParameters::save_state_to_xml(juce::XmlElement& xml)
{
    auto state = apvts_.copyState();
    std::unique_ptr<juce::XmlElement> state_xml(state.createXml());
    
    if (state_xml != nullptr)
    {
        xml.addChildElement(state_xml.release());
        return true;
    }
    
    return false;
}

bool PluginParameters::load_state_from_xml(const juce::XmlElement& xml)
{
    auto* state_xml = xml.getChildByName(apvts_.state.getType());
    
    if (state_xml != nullptr)
    {
        apvts_.replaceState(juce::ValueTree::fromXml(*state_xml));
        return true;
    }
    
    return false;
}

void PluginParameters::reset_to_defaults()
{
    for (const auto& param_def : get_parameter_definitions())
    {
        set_parameter_value(param_def.id, param_def.defaultValue); // defaultValue is real value
    }
}

const std::vector<ParameterDefinition>& PluginParameters::get_parameter_definitions()
{
    static const std::vector<ParameterDefinition> definitions = {
        // Global parameters
        {
            ultraglitch::params::Global_OutputGain, // ID from tasq.md
            "Output Gain",
            "", // No label in tasq.md
            ParameterType::Float,
            0.0f, 2.0f, 0.01f, 1.0f, 1.0f, // Range 0.0-2.0, default 1.0 (linear)
            {}
        },
        {
            ultraglitch::params::Global_ChaosMode, // ID from tasq.md
            "Chaos Mode",
            "", // No label in tasq.md
            ParameterType::Bool,
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // false by default
            {}
        },
        
        // Bit Crusher parameters
        {
            ultraglitch::params::BitCrusher_Enabled, // ID from tasq.md
            "Bitcrusher Enabled",
            "", // No label in tasq.md
            ParameterType::Bool,
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // false by default
            {}
        },
        {
            ultraglitch::params::BitCrusher_BitDepth, // ID from tasq.md
            "Bit Depth",
            "bits",
            ParameterType::Float,
            1.0f, 16.0f, 1.0f, 1.0f, 16.0f, // Range 1-16, default 16
            {}
        },
        {
            ultraglitch::params::BitCrusher_SampleRateDiv, // ID from tasq.md
            "Sample Rate Divisor",
            "", // No label in tasq.md
            ParameterType::Float,
            1.0f, 64.0f, 1.0f, 1.0f, 1.0f, // Range 1-64, default 1
            {}
        },
        {
            ultraglitch::params::BitCrusher_Mix, // ID from tasq.md
            "Bitcrusher Mix",
            "", // No label in tasq.md
            ParameterType::Float,
            0.0f, 1.0f, 0.01f, 1.0f, 0.0f, // Range 0-1, default 0
            {}
        },
        
        // Buffer Stutter parameters
        {
            ultraglitch::params::BufferStutter_Enabled, // ID from tasq.md
            "Stutter Enabled",
            "", // No label in tasq.md
            ParameterType::Bool,
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // false by default
            {}
        },
        {
            ultraglitch::params::BufferStutter_Rate, // ID from tasq.md
            "Stutter Rate",
            "Divisions",
            ParameterType::Float,
            1.0f, 16.0f, 1.0f, 1.0f, 4.0f, // Range 1-16, default 4 divisions
            {}
        },
        {
            ultraglitch::params::BufferStutter_Length, // ID from tasq.md
            "Stutter Length",
            "ms",
            ParameterType::Float,
            10.0f, 500.0f, 1.0f, 0.5f, 100.0f, // Range 10-500ms, default 100ms
            {}
        },
        {
            ultraglitch::params::BufferStutter_Mix, // ID from tasq.md
            "Stutter Mix",
            "", // No label in tasq.md
            ParameterType::Float,
            0.0f, 1.0f, 0.01f, 1.0f, 0.0f, // Range 0-1, default 0
            {}
        },
        
        // Pitch Drift parameters
        {
            ultraglitch::params::PitchDrift_Enabled, // ID from tasq.md
            "Pitch Drift Enabled",
            "", // No label in tasq.md
            ParameterType::Bool,
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // false by default
            {}
        },
        {
            ultraglitch::params::PitchDrift_Amount, // ID from tasq.md
            "Drift Amount",
            "cents",
            ParameterType::Float,
            0.0f, 1200.0f, 1.0f, 1.0f, 0.0f, // Range 0-1200 cents (1 octave), default 0
            {}
        },
        {
            ultraglitch::params::PitchDrift_Speed, // ID from tasq.md
            "Drift Speed",
            "Hz",
            ParameterType::Float,
            0.01f, 10.0f, 0.01f, 0.5f, 1.0f, // Range 0.01-10Hz, default 1.0Hz
            {}
        },
        {
            ultraglitch::params::PitchDrift_Mix, // ID from tasq.md
            "Pitch Drift Mix",
            "", // No label in tasq.md
            ParameterType::Float,
            0.0f, 1.0f, 0.01f, 1.0f, 0.0f, // Range 0-1, default 0
            {}
        },
        
        // Reverse Slice parameters
        {
            ultraglitch::params::ReverseSlice_Enabled, // ID from tasq.md
            "Reverse Slice Enabled",
            "", // No label in tasq.md
            ParameterType::Bool,
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // false by default
            {}
        },
        {
            ultraglitch::params::ReverseSlice_Interval, // ID from tasq.md
            "Slice Interval",
            "ms",
            ParameterType::Float,
            50.0f, 1000.0f, 10.0f, 0.5f, 200.0f, // Range 50-1000ms, default 200ms
            {}
        },
        {
            ultraglitch::params::ReverseSlice_Chance, // ID from tasq.md
            "Reverse Chance",
            "", // No label in tasq.md
            ParameterType::Float,
            0.0f, 1.0f, 0.01f, 1.0f, 0.5f, // Range 0-1, default 0.5
            {}
        },
        {
            ultraglitch::params::ReverseSlice_Mix, // ID from tasq.md
            "Reverse Slice Mix",
            "", // No label in tasq.md
            ParameterType::Float,
            0.0f, 1.0f, 0.01f, 1.0f, 0.0f, // Range 0-1, default 0
            {}
        },
        
        // Slice Rearrange parameters
        {
            ultraglitch::params::SliceRearrange_Enabled, // ID from tasq.md
            "Rearrange Enabled",
            "", // No label in tasq.md
            ParameterType::Bool,
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // false by default
            {}
        },
        {
            ultraglitch::params::SliceRearrange_SliceCount, // ID from tasq.md
            "Slice Count",
            "slices",
            ParameterType::Float,
            2.0f, 16.0f, 1.0f, 1.0f, 4.0f, // Range 2-16, default 4
            {}
        },
        {
            ultraglitch::params::SliceRearrange_Randomize, // ID from tasq.md
            "Randomize Amount",
            "", // No label in tasq.md
            ParameterType::Float,
            0.0f, 1.0f, 0.01f, 1.0f, 0.0f, // Range 0-1, default 0
            {}
        },
        {
            ultraglitch::params::SliceRearrange_Mix, // ID from tasq.md
            "Rearrange Mix",
            "", // No label in tasq.md
            ParameterType::Float,
            0.0f, 1.0f, 0.01f, 1.0f, 0.0f, // Range 0-1, default 0
            {}
        },
        
        // Weird Flanger parameters
        {
            ultraglitch::params::WeirdFlanger_Enabled, // ID from tasq.md
            "Flanger Enabled",
            "", // No label in tasq.md
            ParameterType::Bool,
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // false by default
            {}
        },
        {
            ultraglitch::params::WeirdFlanger_Rate, // ID from tasq.md
            "Flanger Rate",
            "Hz",
            ParameterType::Float,
            0.01f, 20.0f, 0.01f, 0.5f, 1.0f, // Range 0.01-20Hz, default 1.0Hz
            {}
        },
        {
            ultraglitch::params::WeirdFlanger_Depth, // ID from tasq.md
            "Flanger Depth",
            "", // No label in tasq.md
            ParameterType::Float,
            0.0f, 1.0f, 0.01f, 1.0f, 0.8f, // Range 0-1, default 0.8
            {}
        },
        {
            ultraglitch::params::WeirdFlanger_Feedback, // ID from tasq.md
            "Flanger Feedback",
            "", // No label in tasq.md
            ParameterType::Float,
            -1.0f, 1.0f, 0.01f, 1.0f, 0.0f, // Range -1 to 1, default 0
            {}
        },
        {
            ultraglitch::params::WeirdFlanger_Mix, // ID from tasq.md
            "Flanger Mix",
            "", // No label in tasq.md
            ParameterType::Float,
            0.0f, 1.0f, 0.01f, 1.0f, 0.0f, // Range 0-1, default 0
            {}
        },
        
        // Chaos Controller parameters
        // Note: ChaosController is enabled via Global_ChaosMode
        {
            ultraglitch::params::ChaosController_Speed, // ID from tasq.md
            "Chaos Speed",
            "Hz",
            ParameterType::Float,
            0.01f, 10.0f, 0.01f, 0.5f, 4.0f, // Range 0.01-10Hz, default 4.0Hz
            {}
        },
        {
            ultraglitch::params::ChaosController_Intensity, // ID from tasq.md
            "Chaos Intensity",
            "%",
            ParameterType::Float,
            0.0f, 100.0f, 1.0f, 1.0f, 100.0f, // Range 0-100, default 100%
            {}
        }
    };
    return definitions;
}