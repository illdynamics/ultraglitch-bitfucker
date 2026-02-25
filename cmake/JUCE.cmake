function(configure_juce_target target_name)
    if(NOT TARGET juce::juce_core)
        message(FATAL_ERROR "JUCE not available")
    endif()

    target_link_libraries(${target_name} PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    )

    if(APPLE)
        target_link_libraries(${target_name} PRIVATE
            "-framework Accelerate"
            "-framework AudioToolbox"
            "-framework CoreAudio"
            "-framework CoreMIDI"
            "-framework QuartzCore"
        )
    elseif(WIN32)
        target_link_libraries(${target_name} PRIVATE
            winmm
            ws2_32
            ole32
            comctl32
            uuid
        )
    elseif(UNIX AND NOT APPLE)
        find_package(ALSA REQUIRED)
        find_package(X11 REQUIRED)
        find_package(Threads REQUIRED)

        target_link_libraries(${target_name} PRIVATE
            ${ALSA_LIBRARIES}
            ${X11_LIBRARIES}
            Threads::Threads
            dl
            rt
        )
    endif()
endfunction()