#pragma once

#include "PluginProcessor.h"
#include "HandUI.hpp"
#include <mutex>

//==============================================================================
class AudioPluginAudioProcessorEditor final
        : public juce::AudioProcessorEditor, public juce::AudioProcessorParameter::Listener, public juce::AsyncUpdater {
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &);

    ~AudioPluginAudioProcessorEditor() override;

    struct ParameterUpdateMessage {
        int parameterIndex;
        float newValue;
    };
    std::mutex parameter_lock;
    std::vector<ParameterUpdateMessage> parameter_change_messages;

    void parameterValueChanged(int parameterIndex, float newValue) override {
        auto l = std::unique_lock(parameter_lock);
        parameter_change_messages.push_back({parameterIndex, newValue});
        triggerAsyncUpdate();
    }

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {
    }

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

    void handleAsyncUpdate() override {
        auto l = std::unique_lock(parameter_lock);
        for (auto &m: parameter_change_messages) {
            switch (m.parameterIndex) {
                case PARAM_LEFT_HAND_X:
                    left_hand.set_position_x(m.newValue);
                    break;
                case PARAM_LEFT_HAND_Y:
                    left_hand.set_position_y(m.newValue);
                    break;
                case PARAM_LEFT_HAND_Z:
                    left_hand.set_position_z(m.newValue);
                    break;
                case PARAM_RIGHT_HAND_X:
                    right_hand.set_position_x(m.newValue);
                    break;
                case PARAM_RIGHT_HAND_Y:
                    right_hand.set_position_y(m.newValue);
                    break;
                case PARAM_RIGHT_HAND_Z:
                    right_hand.set_position_z(m.newValue);
                    break;
                case PARAM_LEFT_HAND_PINKY:
                    left_hand.toggle_note(0, m.newValue);
                    break;
                case PARAM_LEFT_HAND_RING:
                    left_hand.toggle_note(1, m.newValue);
                    break;
                case PARAM_LEFT_HAND_MIDDLE:
                    left_hand.toggle_note(2, m.newValue);
                    break;
                case PARAM_LEFT_HAND_INDEX:
                    left_hand.toggle_note(3, m.newValue);
                    break;
                case PARAM_RIGHT_HAND_INDEX:
                    right_hand.toggle_note(0, m.newValue);
                    break;
                case PARAM_RIGHT_HAND_MIDDLE:
                    right_hand.toggle_note(1, m.newValue);
                    break;
                case PARAM_RIGHT_HAND_RING:
                    right_hand.toggle_note(2, m.newValue);
                    break;
                case PARAM_RIGHT_HAND_PINKY:
                    right_hand.toggle_note(3, m.newValue);
                    break;
                case PARAM_LEFT_HAND_CC_X:
                    left_hand.set_cc_number_x(m.newValue);
                    break;
                case PARAM_LEFT_HAND_CC_Y:
                    left_hand.set_cc_number_y(m.newValue);
                    break;
                case PARAM_LEFT_HAND_CC_Z:
                    left_hand.set_cc_number_z(m.newValue);
                    break;
                case PARAM_RIGHT_HAND_CC_X:
                    right_hand.set_cc_number_x(m.newValue);
                    break;
                case PARAM_RIGHT_HAND_CC_Y:
                    right_hand.set_cc_number_y(m.newValue);
                    break;
                case PARAM_RIGHT_HAND_CC_Z:
                    right_hand.set_cc_number_z(m.newValue);
                    break;
            }
        }
        parameter_change_messages.clear();
    }

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor &processorRef;
    HandUI left_hand;
    HandUI right_hand;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
