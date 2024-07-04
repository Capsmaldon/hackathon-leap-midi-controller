cmake_minimum_required(VERSION 3.23)
project(hackathon_leap_midi_controller)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FetchContent)

FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 6.1.6
        GIT_SHALLOW TRUE
        )
FetchContent_MakeAvailable(JUCE)

set(LEAP_SDK_DIR ${CMAKE_SOURCE_DIR}/dependencies/LeapSDK)
if(APPLE)
    add_library(LeapSDK SHARED IMPORTED)
    set_property(TARGET LeapSDK PROPERTY IMPORTED_LOCATION "${LEAP_SDK_DIR}/lib/macOS/libLeapC.6.dylib")
    target_include_directories(LeapSDK INTERFACE "${LEAP_SDK_DIR}/include")
else()
    set(LeapSDK_DIR ${LEAP_SDK_DIR}/lib/cmake/LeapSDK)
    find_package(LeapSDK 5 REQUIRED)
endif ()

add_subdirectory(LeapWrapper)

juce_add_plugin(AudioPluginExample
    VERSION 0.0.1                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    # COMPANY_NAME ...                          # Specify the name of the plugin's author
    IS_SYNTH TRUE                       # Is this a synth or an effect?
    # NEEDS_MIDI_INPUT TRUE/FALSE               # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT TRUE                      # Does the plugin need midi output?
    IS_MIDI_EFFECT FALSE                 # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE Juce               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Dem0                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS AU VST3 Standalone                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "Audio Plugin Example")        # The name of the final executable, which can differ from the target name

target_sources(
  AudioPluginExample
  PRIVATE
    PluginEditor.cpp
    PluginProcessor.cpp
    HandUI.cpp
    TestSynthVoice.cpp
    TestSynthVoice.h
    TestSynthSound.h
    InteractionState.cpp
    InteractionState.h
)

target_compile_definitions(AudioPluginExample
  PUBLIC
# JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AudioPluginExample
    PRIVATE
        # AudioPluginData           # If we'd created a binary data target, we'd link to it here
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_audio_devices
        juce::juce_core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        leap_tracker)

