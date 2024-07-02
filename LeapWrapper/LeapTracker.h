#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <shared_mutex>
#include <functional>

#include <LeapC.h>

class LeapTracker
{
private:
    static constexpr auto kFrameBufferSize = 32;

public:
    LeapTracker(std::function<void(std::vector<LEAP_HAND>)> handEvents);
    ~LeapTracker();

private:
    void ServiceMessageLoop();
    void DeviceDetected(uint32_t _, const LEAP_DEVICE_EVENT *event);
    void DeviceLost(uint32_t _, const LEAP_DEVICE_EVENT *event);
    void TrackingFrame(uint32_t deviceId, const LEAP_TRACKING_EVENT *event);

    std::atomic<bool> isRunning = false;
    std::atomic<bool> isConnected = false;

    LEAP_CONNECTION connectionHandle = nullptr;

    std::shared_mutex deviceMutex;
    uint32_t deviceId = -1;
    LEAP_DEVICE deviceHandle = nullptr;
    std::thread serviceThread;

    std::function<void(std::vector<LEAP_HAND>)> callback;
};
