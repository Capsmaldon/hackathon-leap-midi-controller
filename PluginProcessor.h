#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "LeapWrapper/LeapTracker.h"
#include "InteractionState.h"
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>

enum PluginParams
{
    PARAM_LEFT_HAND_X,
    PARAM_LEFT_HAND_Y,
    PARAM_LEFT_HAND_Z,
    PARAM_RIGHT_HAND_X,
    PARAM_RIGHT_HAND_Y,
    PARAM_RIGHT_HAND_Z,
    PARAM_LEFT_HAND_PINKY,
    PARAM_LEFT_HAND_RING,
    PARAM_LEFT_HAND_MIDDLE,
    PARAM_LEFT_HAND_INDEX,
    PARAM_RIGHT_HAND_INDEX,
    PARAM_RIGHT_HAND_MIDDLE,
    PARAM_RIGHT_HAND_RING,
    PARAM_RIGHT_HAND_PINKY,
    PARAM_LEFT_HAND_CC_X,
    PARAM_LEFT_HAND_CC_Y,
    PARAM_LEFT_HAND_CC_Z,
    PARAM_RIGHT_HAND_CC_X,
    PARAM_RIGHT_HAND_CC_Y,
    PARAM_RIGHT_HAND_CC_Z,
};

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    //==============================================================================

private:
    juce::Synthesiser synth;
    juce::dsp::Oscillator<float> osc;

    juce::MidiBuffer internalMidiBuffer;
    juce::CriticalSection internalMidiBufferMutex;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)

    //==============================================================================
    enum class PluginMode
    {
        PINCH_SYNTH,
        PINCH_EXPRESSION,
    };

    PluginMode pluginMode = PluginMode::PINCH_EXPRESSION;

    void leapHandEvent(std::vector<LEAP_HAND> hands);
    void pinchSynthMode(eLeapHandType chirality);
    void pinchExpressionMode(eLeapHandType chirality);

    std::chrono::steady_clock::time_point last_sent_palm_position;

    LeapTracker leapTracker;
    InteractionState interactionState;
    std::unique_ptr<juce::MidiOutput> midiOutput;

    juce::AudioParameterFloat *left_hand_x;
    juce::AudioParameterFloat *left_hand_y;
    juce::AudioParameterFloat *left_hand_z;

    juce::AudioParameterFloat *right_hand_x;
    juce::AudioParameterFloat *right_hand_y;
    juce::AudioParameterFloat *right_hand_z;

    juce::AudioParameterInt *left_hand_pinky;
    juce::AudioParameterInt *left_hand_ring;
    juce::AudioParameterInt *left_hand_middle;
    juce::AudioParameterInt *left_hand_index;

    juce::AudioParameterInt *right_hand_pinky;
    juce::AudioParameterInt *right_hand_ring;
    juce::AudioParameterInt *right_hand_middle;
    juce::AudioParameterInt *right_hand_index;

    juce::AudioParameterInt *left_hand_cc_x;
    juce::AudioParameterInt *left_hand_cc_y;
    juce::AudioParameterInt *left_hand_cc_z;

    juce::AudioParameterInt *right_hand_cc_x;
    juce::AudioParameterInt *right_hand_cc_y;
    juce::AudioParameterInt *right_hand_cc_z;

    //==============================================================================
};
