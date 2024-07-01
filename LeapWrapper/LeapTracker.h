#include <atomic>
#include <thread>
#include <shared_mutex>
#include <deque>

#include <LeapC.h>
#include <vector>


struct HandFrame
{
    LEAP_TRACKING_EVENT header;
    std::vector<LEAP_HAND> hands;
};

class LeapTracker {
private:
    static constexpr auto kFrameBufferSize = 32;

public:
    LeapTracker();
    ~LeapTracker();

    HandFrame GetLatestFrame(eLeapHandType chirality);

private:
    void ServiceMessageLoop();
    void DeviceDetected(uint32_t _, const LEAP_DEVICE_EVENT* event);
    void DeviceLost(uint32_t _, const LEAP_DEVICE_EVENT* event);
    void TrackingFrame(uint32_t deviceId, const LEAP_TRACKING_EVENT* event);

    static bool FrameHasHand(const HandFrame& frame, eLeapHandType chirality);

    std::atomic<bool> isRunning   = false;
    std::atomic<bool> isConnected = false;

    LEAP_CONNECTION connectionHandle = nullptr;

    std::shared_mutex deviceMutex;
    uint32_t deviceId = -1;
    LEAP_DEVICE deviceHandle = nullptr;
    std::thread serviceThread;

    std::shared_mutex framesMutex;
    std::deque<HandFrame> frames;
};
