cmake_minimum_required(VERSION 3.22)

project(AUDIO_PLUGIN_EXAMPLE VERSION 0.0.1)

# -----------------------------
# Override version from CI
# -----------------------------
if(DEFINED PLUGIN_VERSION)
    set(PROJECT_VERSION ${PLUGIN_VERSION})
endif()

add_subdirectory(JUCE)

juce_add_plugin(VBANTX
    COMPANY_NAME "Optimum"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Gyni
    PLUGIN_CODE Plug

    FORMATS VST3
    PRODUCT_NAME "VBANTX"
)

target_sources(VBANTX
    PRIVATE
        PluginProcessor.cpp
)

target_compile_definitions(VBANTX
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(VBANTX
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_generate_juce_header(VBANTX)

# -----------------------------
# Post-build commands to copy the plugin to the dist folder
# -----------------------------
add_custom_target(package_vst3 ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/dist"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_BINARY_DIR}/VBANTX_artefacts/$<CONFIG>/VST3/VBANTX.vst3"
        "${CMAKE_SOURCE_DIR}/dist/VBANTX.vst3"
    DEPENDS VBANTX
    COMMENT "Copying VST3 bundle to dist/"
)