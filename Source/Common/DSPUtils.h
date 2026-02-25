#pragma once

#include <juce_audio_basics/juce_audio_basics.h> // For juce::AudioBuffer
#include <cmath>
#include <algorithm> // For std::clamp

namespace ultraglitch::dsp
{
    // =========================================================================
    // Math utilities
    // =========================================================================
    
    /** Clamps a value between a minimum and maximum. */
    template<typename T>
    inline T clamp(T value, T min_value, T max_value)
    {
        return std::clamp(value, min_value, max_value);
    }

    /** Linearly mixes two values (dry and wet) based on a mix factor (0.0 = dry, 1.0 = wet). */
    template<typename T>
    inline T mix(T dry_value, T wet_value, T mix_factor)
    {
        return dry_value * (1.0f - mix_factor) + wet_value * mix_factor;
    }

    /** Linear interpolation between two values. */
    template<typename T>
    inline T linear_interpolate(T y0, T y1, T t)
    {
        return y0 + t * (y1 - y0);
    }
    
    /** Linear interpolation for arrays with bounds checking. */
    template<typename T>
    inline T linear_interpolate_array(const T* data, int size, float index)
    {
        const int i0 = static_cast<int>(std::floor(index));
        const int i1 = i0 + 1;
        const float t = index - static_cast<float>(i0);
        
        if (i0 < 0) return data[0];
        if (i1 >= size) return data[size - 1];
        
        return linear_interpolate(data[i0], data[i1], t);
    }
    
    // =========================================================================
    // Delay line helpers
    // =========================================================================
    
    /** Circular buffer index calculation with wrap-around. */
    inline int wrap_delay_index(int index, int buffer_size)
    {
        while (index < 0) index += buffer_size;
        while (index >= buffer_size) index -= buffer_size;
        return index;
    }
    
    /** Read from delay line with linear interpolation. */
    inline float read_delay_line(const juce::AudioBuffer<float>& buffer, 
                                 int channel, 
                                 float read_position, 
                                 int buffer_size)
    {
        const int i0 = static_cast<int>(std::floor(read_position));
        const int i1 = wrap_delay_index(i0 + 1, buffer_size);
        const float t = read_position - static_cast<float>(i0);
        
        const float* channel_data = buffer.getReadPointer(channel);
        const float y0 = channel_data[wrap_delay_index(i0, buffer_size)];
        const float y1 = channel_data[i1];
        
        return linear_interpolate(y0, y1, t);
    }
    
    /** Write to delay line with circular indexing. */
    inline void write_delay_line(juce::AudioBuffer<float>& buffer,
                                 int channel,
                                 int write_position,
                                 float value,
                                 int buffer_size)
    {
        float* channel_data = buffer.getWritePointer(channel);
        channel_data[wrap_delay_index(write_position, buffer_size)] = value;
    }
    
    // =========================================================================
    // Frequency and time conversions
    // =========================================================================
    
    /** Convert frequency (Hz) to angular frequency (radians per sample). */
    inline float frequency_to_angular(float frequency_hz, float sample_rate)
    {
        return 2.0f * juce::MathConstants<float>::pi * frequency_hz / sample_rate;
    }
    
    /** Convert time (seconds) to samples. */
    inline int time_to_samples(float time_seconds, float sample_rate)
    {
        return static_cast<int>(std::round(time_seconds * sample_rate));
    }
    
    /** Convert samples to time (seconds). */
    inline float samples_to_time(int num_samples, float sample_rate)
    {
        return static_cast<float>(num_samples) / sample_rate;
    }
    
    /** Convert MIDI note number to frequency (Hz). */
    inline float midi_note_to_frequency(float midi_note)
    {
        return 440.0f * std::pow(2.0f, (midi_note - 69.0f) / 12.0f);
    }
    
    /** Convert frequency (Hz) to MIDI note number. */
    inline float frequency_to_midi_note(float frequency_hz)
    {
        return 69.0f + 12.0f * std::log2(frequency_hz / 440.0f);
    }
    
    // =========================================================================
    // Signal processing utilities
    // =========================================================================
    
    /** Apply soft clipping to a signal. */
    inline float soft_clip(float x, float threshold = 1.0f)
    {
        if (x > threshold)
            return threshold + (x - threshold) / (1.0f + std::abs(x - threshold));
        else if (x < -threshold)
            return -threshold + (x + threshold) / (1.0f + std::abs(x + threshold));
        else
            return x;
    }
    
    /** Apply hard clipping to a signal. */
    inline float hard_clip(float x, float threshold = 1.0f)
    {
        return ultraglitch::dsp::clamp(x, -threshold, threshold); // Use our clamp
    }
    
    /** Apply cubic distortion to a signal. */
    inline float cubic_distortion(float x, float amount = 0.5f)
    {
        return x - amount * x * x * x / 3.0f;
    }
    
    /** Calculate RMS value of a buffer. */
    inline float calculate_rms(const juce::AudioBuffer<float>& buffer, int channel = 0)
    {
        const float* data = buffer.getReadPointer(channel);
        const int num_samples = buffer.getNumSamples();
        
        float sum = 0.0f;
        for (int i = 0; i < num_samples; ++i)
        {
            sum += data[i] * data[i];
        }
        
        return std::sqrt(sum / static_cast<float>(num_samples));
    }
    
    /** Calculate peak value of a buffer. */
    inline float calculate_peak(const juce::AudioBuffer<float>& buffer, int channel = 0)
    {
        const float* data = buffer.getReadPointer(channel);
        const int num_samples = buffer.getNumSamples();
        
        float peak = 0.0f;
        for (int i = 0; i < num_samples; ++i)
        {
            peak = std::max(peak, std::abs(data[i]));
        }
        
        return peak;
    }
    
    // =========================================================================
    // Window functions
    // =========================================================================
    
    /** Calculate Hann window value. */
    inline float hann_window(int n, int N)
    {
        return 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * n / (N - 1)));
    }
    
    /** Calculate Hamming window value. */
    inline float hamming_window(int n, int N)
    {
        return 0.54f - 0.46f * std::cos(2.0f * juce::MathConstants<float>::pi * n / (N - 1));
    }
    
    /** Calculate Blackman window value. */
    inline float blackman_window(int n, int N)
    {
        const float alpha = 0.16f;
        const float a0 = (1.0f - alpha) / 2.0f;
        const float a1 = 0.5f;
        const float a2 = alpha / 2.0f;
        
        return a0 - a1 * std::cos(2.0f * juce::MathConstants<float>::pi * n / (N - 1)) 
                   + a2 * std::cos(4.0f * juce::MathConstants<float>::pi * n / (N - 1));
    }
    
    // =========================================================================
    // Math utilities (continued)
    // =========================================================================
    
    /** Convert decibels to linear gain. */
    inline float decibels_to_gain(float decibels, float minus_infinity_db = -100.0f)
    {
        return decibels > minus_infinity_db ? std::pow(10.0f, decibels * 0.05f) : 0.0f;
    }
    
    /** Convert linear gain to decibels. */
    inline float gain_to_decibels(float gain, float minus_infinity_db = -100.0f)
    {
        return gain > 0.0f ? std::max(minus_infinity_db, 20.0f * std::log10(gain)) : minus_infinity_db;
    }
    
    /** Map value from one range to another. */
    inline float map_value(float value, float in_min, float in_max, float out_min, float out_max)
    {
        return out_min + (out_max - out_min) * (value - in_min) / (in_max - in_min);
    }
    
    /** Apply skew factor to parameter value (for non-linear knobs). */
    inline float apply_skew_factor(float value, float skew_factor)
    {
        if (skew_factor != 0.0f)
        {
            value = std::pow(value, std::exp(skew_factor * std::log(0.5f)));
        }
        return value;
    }
    
    /** Calculate bipolar to unipolar conversion. */
    inline float bipolar_to_unipolar(float bipolar_value)
    {
        return (bipolar_value + 1.0f) * 0.5f;
    }
    
    /** Calculate unipolar to bipolar conversion. */
    inline float unipolar_to_bipolar(float unipolar_value)
    {
        return unipolar_value * 2.0f - 1.0f;
    }
    
    // =========================================================================
    // Crossfade utilities (RT-safe, no allocations)
    // =========================================================================
    
    /** Number of samples for a minimal anti-click crossfade (~0.7ms at 44.1kHz). */
    static constexpr int CROSSFADE_SAMPLES = 32;
    
    /** Apply a linear fade-in ramp to the first N samples of a buffer region. */
    inline void apply_fade_in(float* data, int startSample, int fadeLengthSamples, int totalSamples)
    {
        if (fadeLengthSamples <= 0) return;
        const int fadeEnd = std::min(startSample + fadeLengthSamples, totalSamples);
        for (int i = startSample; i < fadeEnd; ++i)
        {
            float gain = static_cast<float>(i - startSample) / static_cast<float>(fadeLengthSamples);
            data[i] *= gain;
        }
    }
    
    /** Apply a linear fade-out ramp to the last N samples before regionEnd. */
    inline void apply_fade_out(float* data, int regionEnd, int fadeLengthSamples)
    {
        if (fadeLengthSamples <= 0) return;
        const int fadeStart = std::max(0, regionEnd - fadeLengthSamples);
        for (int i = fadeStart; i < regionEnd; ++i)
        {
            float gain = static_cast<float>(regionEnd - i) / static_cast<float>(fadeLengthSamples);
            data[i] *= gain;
        }
    }
    
    /** Apply fade-in and fade-out to a slice within a buffer. */
    inline void apply_slice_crossfade(juce::AudioBuffer<float>& buffer, int startSample, int numSamples, int fadeLen = CROSSFADE_SAMPLES)
    {
        if (numSamples < fadeLen * 2) return; // Slice too short for crossfade
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            float* data = buffer.getWritePointer(ch);
            apply_fade_in(data, startSample, fadeLen, startSample + numSamples);
            apply_fade_out(data, startSample + numSamples, fadeLen);
        }
    }
}