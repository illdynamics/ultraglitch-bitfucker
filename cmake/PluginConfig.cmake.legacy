set(PLUGIN_NAME "Cyqle1")
set(PLUGIN_VERSION "0.1.0")
set(PLUGIN_MANUFACTURER "CyqleAudio")
set(PLUGIN_DESCRIPTION "Glitch Audio Processor")
set(PLUGIN_WEBSITE "https://cyqle.audio")
set(PLUGIN_IS_SYNTH FALSE)
set(PLUGIN_ACCEPTS_MIDI FALSE)
set(PLUGIN_PRODUCES_MIDI FALSE)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginConfig.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/PluginConfig.h
    @ONLY
)

target_include_directories(Cyqle1 PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)