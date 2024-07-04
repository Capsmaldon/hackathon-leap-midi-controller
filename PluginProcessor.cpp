#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "TestSynthVoice.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
      leapTracker{std::bind(&AudioPluginAudioProcessor::leapHandEvent, this, std::placeholders::_1)}
{
#if !JUCE_WINDOWS
    midiOutput = juce::MidiOutput::createNewDevice("Leap Note Tickler");
#endif

    synth.addSound(new TestSynthSound());
    synth.addVoice(new TestSynthVoice());
    last_sent_palm_position = std::chrono::steady_clock::now();

    addParameter(left_hand_x = new juce::AudioParameterFloat("left_hand_x", // parameterID
                                                             "Left Hand X", // parameter name
                                                             0.0f,          // minimum value
                                                             1.0f,          // maximum value
                                                             0.5f));        // default value

    addParameter(left_hand_y = new juce::AudioParameterFloat("left_hand_y", // parameterID
                                                             "Left Hand Y", // parameter name
                                                             0.0f,          // minimum value
                                                             1.0f,          // maximum value
                                                             0.5f));        // default value

    addParameter(left_hand_z = new juce::AudioParameterFloat("left_hand_z", // parameterID
                                                             "Left Hand Z", // parameter name
                                                             0.0f,          // minimum value
                                                             1.0f,          // maximum value
                                                             0.5f));        // default value

    addParameter(right_hand_x = new juce::AudioParameterFloat("right_hand_x", // parameterID
                                                              "Right Hand X", // parameter name
                                                              0.0f,           // minimum value
                                                              1.0f,           // maximum value
                                                              0.5f));         // default value

    addParameter(right_hand_y = new juce::AudioParameterFloat("right_hand_y", // parameterID
                                                              "Right Hand Y", // parameter name
                                                              0.0f,           // minimum value
                                                              1.0f,           // maximum value
                                                              0.5f));         // default value

    addParameter(right_hand_z = new juce::AudioParameterFloat("right_hand_z", // parameterID
                                                              "Right Hand Z", // parameter name
                                                              0.0f,           // minimum value
                                                              1.0f,           // maximum value
                                                              0.5f));         // default value

    addParameter(left_hand_pinky = new juce::AudioParameterInt("left_hand_pinky", // parameterID
                                                               "Left Hand Pinky", // parameter name
                                                               0,
                                                               1,
                                                               0)); // default value

    addParameter(left_hand_ring = new juce::AudioParameterInt("left_hand_ring", // parameterID
                                                              "Left Hand Ring", // parameter name
                                                              0,
                                                              1,
                                                              0));                  // default value
    addParameter(left_hand_middle = new juce::AudioParameterInt("left_hand_middle", // parameterID
                                                                "Left Hand Middle", // parameter name
                                                                0,
                                                                1,
                                                                0));              // default value
    addParameter(left_hand_index = new juce::AudioParameterInt("left_hand_index", // parameterID
                                                               "Left Hand Index", // parameter name
                                                               0,
                                                               1,
                                                               0)); // default value

    addParameter(right_hand_index = new juce::AudioParameterInt("right_hand_index", // parameterID
                                                                "Right Hand Index", // parameter name
                                                                0,
                                                                1,
                                                                0)); // default value

    addParameter(right_hand_middle = new juce::AudioParameterInt("right_hand_middle", // parameterID
                                                                 "Right Hand Middle", // parameter name
                                                                 0,
                                                                 1,
                                                                 0));             // default value
    addParameter(right_hand_ring = new juce::AudioParameterInt("right_hand_ring", // parameterID
                                                               "Right Hand Ring", // parameter name
                                                               0,
                                                               1,
                                                               0));                 // default value
    addParameter(right_hand_pinky = new juce::AudioParameterInt("right_hand_pinky", // parameterID
                                                                "Right Hand Pinky", // parameter name
                                                                0,
                                                                1,
                                                                0));
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::ignoreUnused(sampleRate, samplesPerBlock);
    juce::dsp::ProcessSpec spec{sampleRate, static_cast<uint32_t>(samplesPerBlock),
                                static_cast<uint32_t>(getTotalNumOutputChannels())};

    synth.setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<TestSynthVoice *>(synth.getVoice(i)))
        {
            voice->prepareToPlay(spec);
        }
    }

    // synth.noteOn(1, 48, uint8_t(127));
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    // juce::dsp::AudioBlock<float> synthBlock{buffer};
    if (midiOutput)
    {
        synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    }
    else
    {
        juce::ScopedLock sl (internalMidiBufferMutex);
        internalMidiBuffer.addEvents(midiMessages, 0, -1, 0);
        synth.renderNextBlock(buffer, internalMidiBuffer, 0, buffer.getNumSamples());
        internalMidiBuffer.clear();
    }

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto *channelData = buffer.getWritePointer(channel);
        juce::ignoreUnused(channelData);
        // ..do something to the data...
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused(destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
void AudioPluginAudioProcessor::leapHandEvent(std::vector<LEAP_HAND> hands)
{
    for (const auto &hand : hands)
    {
        interactionState.updateHandState(hand);

        switch (pluginMode)
        case PluiginMode::PINCH_SYNTH:
        default:
            pinchSynthMode(hand.type);
    }
}

void AudioPluginAudioProcessor::pinchSynthMode(eLeapHandType chirality)
{
    // Grab the latest update of the hand state.
    const auto handState = interactionState.getHandState(chirality);

    // Handle palm X Y and Z.
    const auto palmCCs =
            chirality == eLeapHandType_Left ? std::array<int, 3>{34, 35, 36} : std::array<int, 3>{37, 38, 39};

    // Get invLerps of palm positions.
    const auto palmPos = std::array<float, 3>{
        inverseLerp(-200.f, 200.f, handState.palmPosition.x),
        inverseLerp(100.f, 300.f, handState.palmPosition.y),
        inverseLerp(-200.f, 200.f, handState.palmPosition.z)};

    auto now = std::chrono::steady_clock::now();
    if ((now - last_sent_palm_position) > std::chrono::milliseconds(50))
    {
        last_sent_palm_position = now;
        for (const auto i : {0, 1, 2})
        {
            int value = palmPos[i] * 127;
            value = std::clamp(value, 0, 127);
            auto msg = juce::MidiMessage::controllerEvent(1, palmCCs[i], value);
            if (midiOutput)
            {
                midiOutput->sendMessageNow(msg);
            }
            else
            {
                juce::ScopedLock sl (internalMidiBufferMutex);
                internalMidiBuffer.addEvent(msg, 0);
            }
        }
    }

    // Update the UI to show the palm representation.
    if (chirality == eLeapHandType_Left)
    {
        left_hand_x->setValueNotifyingHost(std::clamp(palmPos[0], 0.0f, 1.0f));
        left_hand_y->setValueNotifyingHost(std::clamp(palmPos[1], 0.0f, 1.0f));
        left_hand_z->setValueNotifyingHost(std::clamp(palmPos[2], 0.0f, 1.0f));
    }
    else
    {
        right_hand_x->setValueNotifyingHost(std::clamp(palmPos[0], 0.0f, 1.0f));
        right_hand_y->setValueNotifyingHost(std::clamp(palmPos[1], 0.0f, 1.0f));
        right_hand_z->setValueNotifyingHost(std::clamp(palmPos[2], 0.0f, 1.0f));
    }

    const auto noteEvent = [&](const auto &state, int noteNumber)
    {
        auto msg = juce::MidiMessage{};
        if (state)
        {
            msg = juce::MidiMessage::noteOn(1, noteNumber, 1.0f);
        }
        else
        {
            msg = juce::MidiMessage::noteOff(1, noteNumber, 1.0f);
        }

        if (midiOutput)
        {
            midiOutput->sendMessageNow(msg);
        }
        else
        {
            juce::ScopedLock sl (internalMidiBufferMutex);
            internalMidiBuffer.addEvent(msg, 0);
        }
    };

    int notes[8]{60, 62, 64, 67, 72, 74, 76, 79};

    if (handState.pinky.hasChanged)
    {
        if (chirality == eLeapHandType_Left)
        {
            noteEvent(handState.pinky.isPinching, notes[0]);
            left_hand_pinky->setValueNotifyingHost(handState.pinky.isPinching ? 1 : 0);
        }
        else
        {
            noteEvent(handState.pinky.isPinching, notes[7]);
            right_hand_pinky->setValueNotifyingHost(handState.pinky.isPinching ? 1 : 0);
        }
    }

    if (handState.ring.hasChanged)
    {
        if (chirality == eLeapHandType_Left)
        {
            noteEvent(handState.ring.isPinching, notes[1]);
            left_hand_ring->setValueNotifyingHost(handState.ring.isPinching ? 1 : 0);
        }
        else
        {
            noteEvent(handState.ring.isPinching, notes[6]);
            right_hand_ring->setValueNotifyingHost(handState.ring.isPinching ? 1 : 0);
        }
    }

    if (handState.middle.hasChanged)
    {
        if (chirality == eLeapHandType_Left)
        {
            noteEvent(handState.middle.isPinching, notes[2]);
            left_hand_middle->setValueNotifyingHost(handState.middle.isPinching ? 1 : 0);
        }
        else
        {
            noteEvent(handState.middle.isPinching, notes[5]);
            right_hand_middle->setValueNotifyingHost(handState.middle.isPinching ? 1 : 0);
        }
    }

    if (handState.index.hasChanged)
    {
        if (chirality == eLeapHandType_Left)
        {
            noteEvent(handState.index.isPinching, notes[3]);
            left_hand_index->setValueNotifyingHost(handState.index.isPinching ? 1 : 0);
        }
        else
        {
            noteEvent(handState.index.isPinching, notes[4]);
            right_hand_index->setValueNotifyingHost(handState.index.isPinching ? 1 : 0);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
