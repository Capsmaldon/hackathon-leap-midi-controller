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
    for (int i = 0; i < 8; ++i)
    {
        synth.addVoice(new TestSynthVoice());
    }

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
        juce::ScopedLock sl(internalMidiBufferMutex);
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
        switch (pluginMode)
        case PluiginMode::PINCH_SYNTH:
        default:
            pinchSynthMode(hand);
    }
}

void AudioPluginAudioProcessor::pinchSynthMode(const LEAP_HAND &hand)
{
    // Handle palm X Y and Z.
    const auto palmCCs =
        hand.type == eLeapHandType_Left ? std::array<int, 3>{34, 35, 36} : std::array<int, 3>{37, 38, 39};

    // Get invLerps of palm positions.

    const auto palmPos_left = std::array<float, 3>{
        invLerp(-300.f, 0.f, hand.palm.position.x),
        invLerp(100.f, 300.f, hand.palm.position.y),
        invLerp(-200.f, 200.f, hand.palm.position.z)};

    const auto palmPos_right = std::array<float, 3>{
        invLerp(0.f, 300.f, hand.palm.position.x),
        invLerp(100.f, 300.f, hand.palm.position.y),
        invLerp(-200.f, 200.f, hand.palm.position.z)};

    const auto &palmPos = hand.type == eLeapHandType_Left ? palmPos_left : palmPos_right;

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
                juce::ScopedLock sl(internalMidiBufferMutex);
                internalMidiBuffer.addEvent(msg, 0);
            }
        }
    }

    // Update the UI to show the palm representation.
    if (hand.type == eLeapHandType_Left)
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

    // Individual finger pinch calcs
    const auto PinchStateChanged = [&](bool &state, float value)
    {
        auto currentState = state;

        if (!currentState && value > triggerThreshold)
        {
            state = true;
            return true;
        }
        else if (currentState && value < releaseThreshold)
        {
            state = false;
            return true;
        }

        return false;
    };

    auto &fingerPinches = hand.type == eLeapHandType_Left ? previousPinches[0] : previousPinches[1];
    const auto pinkyChanged = PinchStateChanged(
        fingerPinches.pinky,
        calculatePinch(hand.thumb.distal.next_joint, hand.pinky.distal.next_joint));
    const auto ringChanged = PinchStateChanged(
        fingerPinches.ring,
        calculatePinch(hand.thumb.distal.next_joint, hand.ring.distal.next_joint));
    const auto middleChanged = PinchStateChanged(
        fingerPinches.middle,
        calculatePinch(hand.thumb.distal.next_joint, hand.middle.distal.next_joint));
    const auto indexChanged = PinchStateChanged(
        fingerPinches.index,
        calculatePinch(hand.thumb.distal.next_joint, hand.index.distal.next_joint));

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
            juce::ScopedLock sl(internalMidiBufferMutex);
            internalMidiBuffer.addEvent(msg, 0);
        }
    };

    int major[7]{0, 2, 4, 5, 7, 9, 11};
    int offset = 52;
    int left_hand_notes[8]{
        offset + major[2],
        offset + major[3],
        offset + major[4],
        offset + major[5],
        offset + major[6],
        offset + major[0] + 12,
        offset + major[1] + 12};
    offset = 64;
    int right_hand_notes[8]{
        offset + major[2],
        offset + major[3],
        offset + major[4],
        offset + major[5],
        offset + major[6],
        offset + major[0] + 12,
        offset + major[1] + 12};
    bool finger_states[8]{previousPinches[0].pinky,
                          previousPinches[0].ring,
                          previousPinches[0].middle,
                          previousPinches[0].index,
                          previousPinches[1].index,
                          previousPinches[1].middle,
                          previousPinches[1].ring,
                          previousPinches[1].pinky};
    juce::AudioParameterInt *finger_params[8]{left_hand_pinky,
                                              left_hand_ring,
                                              left_hand_middle,
                                              left_hand_index,
                                              right_hand_index,
                                              right_hand_middle,
                                              right_hand_ring,
                                              right_hand_pinky};
    bool left_hand = hand.type == eLeapHandType_Left;
    bool finger_state_changes[8]{left_hand && pinkyChanged,
                                 left_hand && ringChanged,
                                 left_hand && middleChanged,
                                 left_hand && indexChanged,
                                 !left_hand && indexChanged,
                                 !left_hand && middleChanged,
                                 !left_hand && ringChanged,
                                 !left_hand && pinkyChanged};

    int note_index_shift = static_cast<int>(std::clamp(palmPos[1], 0.0f, 1.0f) + 0.5f) * 3;

    // Left hand - pinky finger priority
    int left_playing_finger_index = -1;
    for (int i = 0; i < 4; ++i)
    {
        if (finger_states[i])
        {
            left_playing_finger_index = i;
            break;
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        if (finger_state_changes[i])
        {
            if (last_left_playing_finger.first != -1)
            {
                noteEvent(false, last_left_playing_finger.second);
                finger_params[last_left_playing_finger.first]->setValueNotifyingHost(0);
            }

            int note_num = left_hand_notes[i + note_index_shift];
            noteEvent(finger_states[i], note_num);
            std::cout << note_num << std::endl;
            finger_params[i]->setValueNotifyingHost(finger_states[i] ? 1 : 0);
            last_left_playing_finger = {left_playing_finger_index, left_hand_notes[i + note_index_shift]};
            break;
        }
    }

    // Right hand - pinky finger priority
    int right_playing_finger_index = -1;
    for (int i = 7; i >= 4; --i)
    {
        if (finger_states[i])
        {
            right_playing_finger_index = i;
            break;
        }
    }

    for (int i = 7; i >= 4; --i)
    {
        if (finger_state_changes[i])
        {
            if (last_right_playing_finger.first != -1)
            {
                noteEvent(false, last_right_playing_finger.second);
                finger_params[last_right_playing_finger.first]->setValueNotifyingHost(0);
            }
            int note_num = right_hand_notes[(i - 4) + note_index_shift];
            std::cout << note_num << std::endl;
            noteEvent(finger_states[i], note_num);
            finger_params[i]->setValueNotifyingHost(finger_states[i] ? 1 : 0);
            last_right_playing_finger = {right_playing_finger_index, right_hand_notes[(i - 4) + note_index_shift]};
            break;
        }
    }
}

float AudioPluginAudioProcessor::calculatePinch(const LEAP_VECTOR &thumbTip, const LEAP_VECTOR &fingerTip)
{
    const auto vecLength = [](const LEAP_VECTOR &v)
    {
        return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    };

    static constexpr auto fullPinch = 15.0f;    // 15mms dist is considered a full pinch.
    static constexpr auto fullRelease = 100.0f; // 100mms dist between fingers is considered a full release

    LEAP_VECTOR distVec = {thumbTip.x - fingerTip.x, thumbTip.y - fingerTip.y, thumbTip.z - fingerTip.z};
    return invLerp(fullRelease, fullPinch, vecLength(distVec));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
