cmake_minimum_required(VERSION 3.22)

project(AUDIO_PLUGIN_EXAMPLE VERSION 0.0.1)

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
    PRODUCT_NAME "VBAN TX")

juce_generate_juce_header(VBANTX)

target_sources(VBANTX
    PRIVATE
        PluginProcessor.cpp)

target_compile_definitions(VBANTX
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(VBANTX
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
