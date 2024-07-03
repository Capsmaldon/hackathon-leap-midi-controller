//
// Created by Luke Saxton on 02/07/2024.
//

#include "TestSynthVoice.h"

TestSynthVoice::TestSynthVoice()
{
    //    osc.initialise([](float x) { return std::sin(x); }
    osc.initialise([](float x)
                   { return x / juce::MathConstants<float>::pi; });
}

bool TestSynthVoice::canPlaySound(juce::SynthesiserSound *sound)
{
    return dynamic_cast<juce::SynthesiserSound *>(sound) != nullptr;
}

void TestSynthVoice::startNote(int midiNoteNumber,
                               float velocity,
                               juce::SynthesiserSound *sound,
                               int currentPitchWheelPosition)
{
    osc.setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
    adsr.noteOn();
}

void TestSynthVoice::stopNote(float velocity, bool allowTailOff)
{
    adsr.noteOff();
}

void TestSynthVoice::pitchWheelMoved(int newPitchWheelValue)
{
}

void TestSynthVoice::controllerMoved(int controllerNumber, int newControllerValue)
{
    // cutoff
    if (controllerNumber == 34)
    {
        auto newFc = (newControllerValue + 1) / 127.0f;
        newFc = newFc * newFc;
        ladder.setCutoffFrequencyHz(newFc * 20000.0f);
    }
    // reso
    else if (controllerNumber == 35)
    {
        ladder.setResonance(newControllerValue / 127.0f);
    }
}

void TestSynthVoice::aftertouchChanged(int newAftertouchValue)
{
}

void TestSynthVoice::channelPressureChanged(int newChannelPressureValue)
{
}

void TestSynthVoice::prepareToPlay(juce::dsp::ProcessSpec &spec)
{
    osc.prepare(spec);
    osc.setFrequency(220);

    ladder.prepare(spec);
    adsr.setSampleRate(spec.sampleRate);
}

void TestSynthVoice::renderNextBlock(juce::AudioBuffer<float> &outputBuffer,
                                     int startSample,
                                     int numSamples)
{
    synthBuffer.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
    synthBuffer.clear();

    auto synthBlock = juce::dsp::AudioBlock<float>(synthBuffer);

    juce::dsp::ProcessContextReplacing<float> context(synthBlock);
    osc.process(context);
    ladder.process(context);
    adsr.applyEnvelopeToBuffer(synthBuffer, 0, numSamples);

    for (int c = 0; c < outputBuffer.getNumChannels(); c++)
    {
        outputBuffer.addFrom(c, startSample, synthBuffer, c, 0, synthBuffer.getNumSamples());
    }
}
