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

    synth.noteOn(1, 48, uint8_t(127));
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
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

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
void AudioPluginAudioProcessor::leapHandEvent(std::vector<LEAP_HAND> hands) {
    for (const auto &hand: hands) {
        processHand(hand);
    }
}

void AudioPluginAudioProcessor::processHand(const LEAP_HAND& hand)
{
    const auto invLerp = [](float lower, float upper, float value)
    {
        return (value - lower) / (upper - lower);
    };

    // Handle palm X Y and Z.
    const auto palmCCs =
        hand.type == eLeapHandType_Left ? std::array<int, 3>{60, 61, 62} : std::array<int, 3>{63, 64, 65};

    // Get invLerps of palm positions.
    const auto palmPos = std::array<float, 3>{
        invLerp(-200.f, 200.f, hand.palm.position.x),
        invLerp(-200.f, 200.f, hand.palm.position.y),
        invLerp(-200.f, 200.f, hand.palm.position.z)
    };

    for (const auto i : {0, 1, 2})
    {
        int value = palmPos[i] * 127;
        value = std::clamp(value, 0, 127);
        auto msg = juce::MidiMessage::controllerEvent(1, palmCCs[i], value);
        if (midiOutput)
        {
            midiOutput->sendMessageNow(msg);
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
    const auto PinchStateChanged = [&](bool& state, float value)
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

    auto& fingerPinches = hand.type == eLeapHandType_Left ? previousPinches[0] : previousPinches[1];
    const auto pinkyChanged = PinchStateChanged(
        fingerPinches.pinky,
        calculatePinch(hand.thumb.distal.next_joint,hand.pinky.distal.next_joint));
    const auto ringChanged = PinchStateChanged(
        fingerPinches.ring,
        calculatePinch(hand.thumb.distal.next_joint, hand.ring.distal.next_joint));
    const auto middleChanged = PinchStateChanged(
        fingerPinches.middle,
        calculatePinch(hand.thumb.distal.next_joint, hand.middle.distal.next_joint));
    const auto indexChanged = PinchStateChanged(
        fingerPinches.index,
        calculatePinch(hand.thumb.distal.next_joint, hand.index.distal.next_joint));

    const auto noteEvent = [&](const auto& state, int noteNumber)
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
    };

    if (pinkyChanged)
    {
        if (hand.type == eLeapHandType_Left)
        {
            noteEvent(fingerPinches.pinky, 60);
        }
        else
        {
            noteEvent(fingerPinches.pinky, 67);
        }
    }

    if (ringChanged)
    {
        if (hand.type == eLeapHandType_Left)
        {
            noteEvent(fingerPinches.ring, 61);
        }
        else
        {
            noteEvent(fingerPinches.ring, 66);
        }
    }

    if (middleChanged)
    {
        if (hand.type == eLeapHandType_Left)
        {
            noteEvent(fingerPinches.middle, 62);
        }
        else
        {
            noteEvent(fingerPinches.middle, 65);
        }
    }

    if (indexChanged)
    {
        if (hand.type == eLeapHandType_Left)
        {
            noteEvent(fingerPinches.index, 63);
        }
        else
        {
            noteEvent(fingerPinches.index, 64);
        }
    }
}

float AudioPluginAudioProcessor::calculatePinch(const LEAP_VECTOR& thumbTip, const LEAP_VECTOR& fingerTip)
{
    const auto vecLength = [](const LEAP_VECTOR& v)
    {
        return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    };

    LEAP_VECTOR tipDistance = {thumbTip.x - fingerTip.x, thumbTip.y - fingerTip.y, thumbTip.z - fingerTip.z};
    return vecLength(tipDistance);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
