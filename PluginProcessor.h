#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "LeapWrapper/LeapTracker.h"
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
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)

    //==============================================================================
    struct FingerPinches
    {
        bool pinky = false;
        bool ring = false;
        bool middle = false;
        bool index = false;
    };

    void leapHandEvent(std::vector<LEAP_HAND> hands);
    void processHand(const LEAP_HAND& hand);
    float calculatePinch(const LEAP_VECTOR& thumbTip, const LEAP_VECTOR& fingerTip);

    LeapTracker leapTracker;
    std::unique_ptr<juce::MidiOutput> midiOutput;
    std::array<FingerPinches, 2> previousPinches;
    const float triggerThreshold = 0.8f;
    const float releaseThreshold = 0.6f;

    juce::AudioParameterFloat *left_hand_x;
    juce::AudioParameterFloat *left_hand_y;
    juce::AudioParameterFloat *left_hand_z;

    juce::AudioParameterFloat *right_hand_x;
    juce::AudioParameterFloat *right_hand_y;
    juce::AudioParameterFloat *right_hand_z;

    //==============================================================================
};
