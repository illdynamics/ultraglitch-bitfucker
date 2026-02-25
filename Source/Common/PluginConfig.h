#ifndef PLUGINCONFIG_H
#define PLUGINCONFIG_H

#include <juce_core/juce_core.h>

namespace ultraglitch::config
{
    // Plugin constants
    const juce::String PLUGIN_NAME = "UltraGlitch BitFucker";
    const juce::String PLUGIN_VERSION_STRING = "0.3.0-beta";
    const int PLUGIN_VERSION_MAJOR = 0;
    const int PLUGIN_VERSION_MINOR = 3;
    const int PLUGIN_VERSION_PATCH = 0;

    const juce::String PLUGIN_MANUFACTURER = "UltraGlitch Audio";
    const juce::String PLUGIN_DESCRIPTION = "Multi-effect audio plugin for creative glitch production";
    
    // Plugin format support
    const bool SUPPORTS_VST3 = true;
    const bool SUPPORTS_AU = true;
    const bool SUPPORTS_STANDALONE = true; // Changed to true
    
    // Audio configuration
    const int MAX_NUM_CHANNELS = 2;
    const double DEFAULT_SAMPLE_RATE = 44100.0;
    const int DEFAULT_BLOCK_SIZE = 512; // This is a default, not a max
    const int MAX_BLOCK_SIZE = 8192; // Added as per tasq.md golden path test
    
    // Effect chain configuration
    const int MAX_EFFECTS_IN_CHAIN = 8;
    const int NUM_EFFECT_MODULES = 7;
    
    // GUI configuration
    const int EDITOR_WIDTH = 800;
    const int EDITOR_HEIGHT = 600;
    const int MIN_EDITOR_WIDTH = 400;
    const int MIN_EDITOR_HEIGHT = 300;
    
    // Parameter configuration
    const float PARAMETER_SMOOTHING_TIME_MS = 50.0f;
    
    // Utility functions
    inline juce::String get_plugin_name() { return PLUGIN_NAME; }
    inline juce::String get_plugin_version_string() { return PLUGIN_VERSION_STRING; }
    inline juce::String get_manufacturer_name() { return PLUGIN_MANUFACTURER; }
    
    inline bool is_valid_channel_count(int numChannels)
    {
        return numChannels > 0 && numChannels <= MAX_NUM_CHANNELS;
    }
    
    inline bool is_valid_sample_rate(double sampleRate)
    {
        return sampleRate > 0.0 && sampleRate <= 192000.0;
    }
    
    inline bool is_valid_block_size(int blockSize)
    {
        return blockSize > 0 && blockSize <= MAX_BLOCK_SIZE; // Using MAX_BLOCK_SIZE
    }
}

#endif // PLUGINCONFIG_H