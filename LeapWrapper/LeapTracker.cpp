#include "LeapTracker.h"

#include <algorithm>

LeapTracker::LeapTracker()
{
    LeapOpenConnection(connectionHandle);

    // Start the service thread.
    isRunning = true;
    serviceThread = std::thread{&LeapTracker::ServiceMessageLoop, this};
}

LeapTracker::~LeapTracker()
{
    {
        std::unique_lock lock{deviceMutex};

        if (deviceHandle != nullptr)
        {
            LeapCloseDevice(deviceHandle);
        }
    }


    isRunning = false;
    LeapCloseConnection(connectionHandle);

    if (serviceThread.joinable())
    {
        serviceThread.join();
    }

    LeapDestroyConnection(connectionHandle);
}

HandFrame LeapTracker::GetLatestFrame(eLeapHandType chirality)
{
    std::shared_lock lock{framesMutex};

    const auto search = std::find_if(std::rbegin(frames), std::rend(frames), [&](const HandFrame& frame){
        return FrameHasHand(frame, chirality);
    });

    if (search != std::rend(frames))
    {
        return *search;
    }

    return HandFrame{};
}

void LeapTracker::ServiceMessageLoop()
{
    LEAP_CONNECTION_MESSAGE msg;

    while (isRunning)
    {
        eLeapRS result = LeapPollConnection(connectionHandle, 100, &msg);
        if (LEAP_FAILED(result))
        {
            continue;
        }

        switch (msg.type)
        {
        case eLeapEventType_Connection: isConnected = true;
            break;
        case eLeapEventType_ConnectionLost: isConnected = false;
            break;
        case eLeapEventType_Device: DeviceDetected(msg.device_id, msg.device_event);
            break;
        case eLeapEventType_DeviceLost: DeviceLost(msg.device_id, msg.device_event);
            break;
        case eLeapEventType_Tracking: TrackingFrame(msg.device_id, msg.tracking_event);
            break;
        default: continue;
        }
    }
}

void LeapTracker::DeviceDetected(uint32_t _, const LEAP_DEVICE_EVENT* event)
{
    std::unique_lock lock{deviceMutex};

    if (deviceHandle != nullptr)
    {
        LeapCloseDevice(deviceHandle);
    }

    LeapOpenDevice(event->device, &deviceHandle);
    LeapSubscribeEvents(connectionHandle, deviceHandle);
}

void LeapTracker::DeviceLost(uint32_t _, const LEAP_DEVICE_EVENT* event)
{
    std::unique_lock lock{deviceMutex};

    if (event->device.id == deviceId)
    {
        LeapCloseDevice(deviceHandle);
        deviceHandle = nullptr;
        deviceId = -1;
    }
}

void LeapTracker::TrackingFrame(uint32_t deviceId, const LEAP_TRACKING_EVENT* event)
{
    if (deviceId != this->deviceId)
    {
        return;
    }

    HandFrame frame{};
    frame.header = *event;
    frame.header.pHands = frame.hands.data();

    frame.hands.resize(frame.header.nHands);
    std::copy_n(event->pHands, event->nHands, frame.header.pHands);

    {
        std::unique_lock lock{framesMutex};
        frames.push_back(std::move(frame));

        while (frames.size() > kFrameBufferSize)
        {
            frames.pop_front();
        }
    }
}

bool LeapTracker::FrameHasHand(const HandFrame& frame, eLeapHandType chirality)
{
    for (const auto& hand : frame.hands)
    {
        if (hand.type == chirality)
        {
            return true;
        }
    }

    return false;
}
