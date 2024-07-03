#pragma once

#include "PluginProcessor.h"
#include "HandUI.hpp"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor, public juce::AudioProcessorParameter::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &);
    ~AudioPluginAudioProcessorEditor() override;
    void parameterValueChanged(int parameterIndex, float newValue) override
    {
        switch (parameterIndex)
        {
        case PARAM_LEFT_HAND_X:
            left_hand.set_position_x(newValue);
            break;
        case PARAM_LEFT_HAND_Y:
            left_hand.set_position_y(newValue);
            break;
        case PARAM_LEFT_HAND_Z:
            left_hand.set_position_z(newValue);
            break;
        case PARAM_RIGHT_HAND_X:
            right_hand.set_position_x(newValue);
            break;
        case PARAM_RIGHT_HAND_Y:
            right_hand.set_position_y(newValue);
            break;
        case PARAM_RIGHT_HAND_Z:
            right_hand.set_position_z(newValue);
            break;
        }
    }
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override
    {
    }

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor &processorRef;
    HandUI left_hand;
    HandUI right_hand;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
