//
// Created by Luke Saxton on 02/07/2024.
//

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

class TestSynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int midiNoteNumber) override
    {
        return true;
    }

    virtual bool appliesToChannel (int midiChannel) override
    {
        return true;
    }
};
