//
// Created by Luke Saxton on 02/07/2024.
//

#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "TestSynthSound.h"

class TestSynthVoice : public juce::SynthesiserVoice
{

public:
    TestSynthVoice();

    bool canPlaySound(juce::SynthesiserSound *) override;

    void startNote(int midiNoteNumber,
                   float velocity,
                   juce::SynthesiserSound *sound,
                   int currentPitchWheelPosition) override;

    void stopNote(float velocity, bool allowTailOff) override;

    void pitchWheelMoved(int newPitchWheelValue) override;

    void controllerMoved(int controllerNumber, int newControllerValue) override;

    void aftertouchChanged(int newAftertouchValue) override;

    void channelPressureChanged(int newChannelPressureValue) override;

    void prepareToPlay(juce::dsp::ProcessSpec &spec);

    void renderNextBlock(juce::AudioBuffer<float> &outputBuffer,
                         int startSample,
                         int numSamples) override;

private:
    juce::dsp::Oscillator<float> osc;
    juce::dsp::LadderFilter<float> ladder;
    juce::ADSR adsr;

    juce::AudioBuffer<float> synthBuffer;
};
