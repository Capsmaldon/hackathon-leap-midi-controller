#include "LeapTracker.h"

#include <vector>

LeapTracker::LeapTracker(std::function<void(std::vector<LEAP_HAND>)> handEvents) : callback{std::move(handEvents)}
{
    LEAP_CONNECTION_CONFIG config{
        sizeof(LEAP_CONNECTION_CONFIG),
        eLeapConnectionConfig_MultiDeviceAware,
        nullptr, // Leave server namespace as default.
        eLeapTrackingOrigin::eLeapTrackingOrigin_DeviceCenter,
    };

    LeapCreateConnection(&config, &connectionHandle);
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
    // Copy out the leap hands from the event and fire them off through the callback given on construction.
    std::vector<LEAP_HAND> hands;
    hands.resize(event->nHands);
    std::copy_n(event->pHands, event->nHands, hands.data());
    callback(std::move(hands));
}
