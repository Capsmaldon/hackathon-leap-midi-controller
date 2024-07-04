#include <array>

#include <LeapC.h>

static float inverseLerp(float lower, float upper, float value)
{
    return (value - lower) / (upper - lower);
};

class InteractionState {

public:
    struct FingerState {
        bool hasChanged = false;
        bool isPinching = false;
        LEAP_VECTOR palmStartOfPinch{};
        LEAP_VECTOR pinchPosDelta{};
    };

    struct HandState {
        LEAP_VECTOR palmPosition{};
        FingerState pinky{};
        FingerState ring{};
        FingerState middle{};
        FingerState index{};
    };

private:
    static constexpr float triggerThreshold = 0.8f;
    static constexpr float releaseThreshold = 0.6f;

public:
    InteractionState() = default;
    ~InteractionState() = default;


    void updateHandState(const LEAP_HAND& hand);
    HandState getHandState(eLeapHandType chirality);

private:
    static float calculatePinch(const LEAP_VECTOR &thumbTip, const LEAP_VECTOR &fingerTip);
    static bool pinchStateChanged(bool& currentPinchState, float pinchValue);

    std::array<HandState, 2> handStates;
};
