#include "InteractionState.h"

#include <cmath>

void InteractionState::updateHandState(const LEAP_HAND &hand) {
    auto& handState = hand.type == eLeapHandType_Left ? handStates[0] : handStates[1];


    // Firstly update palm position
    handState.palmPosition = hand.palm.position;

    // Next look through each finger, calculate pinch and if state has changed, update accordingly.
    const auto& thumbTip = hand.thumb.distal.next_joint;
    const auto CalculateFinger = [&](FingerState &fingerState, const LEAP_VECTOR &fingerTip) {
        // First update changed state and whether the finger is pinching or not.
        fingerState.hasChanged = pinchStateChanged(fingerState.isPinching, calculatePinch(thumbTip, fingerTip));

        // If state has changed and its pinching then we need to set our origin of the palm position.
        if (fingerState.hasChanged && fingerState.isPinching) {
            fingerState.palmStartOfPinch = hand.palm.position;
            fingerState.pinchPosDelta = LEAP_VECTOR{};
        }

        // If the state hasn't changed, and we're still pinching then update the delta.
        if (!fingerState.hasChanged && fingerState.isPinching) {
            fingerState.pinchPosDelta = {handState.palmPosition.x - fingerState.palmStartOfPinch.x,
                                         handState.palmPosition.y - fingerState.palmStartOfPinch.y,
                                         handState.palmPosition.z - fingerState.palmStartOfPinch.z};
        }
    };

    // Calculate all the fingers states;
    CalculateFinger(handState.pinky, hand.pinky.distal.next_joint);
    CalculateFinger(handState.ring, hand.ring.distal.next_joint);
    CalculateFinger(handState.middle, hand.middle.distal.next_joint);
    CalculateFinger(handState.index, hand.index.distal.next_joint);
}

InteractionState::HandState InteractionState::getHandState(eLeapHandType chirality) {
    return chirality == eLeapHandType_Left ? handStates[0] : handStates[1];
}

float InteractionState::calculatePinch(const LEAP_VECTOR &thumbTip, const LEAP_VECTOR &fingerTip) {
    const auto vecLength = [](const LEAP_VECTOR &v)
    {
        return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    };

    static constexpr auto fullPinch = 15.0f;    // 15mms dist is considered a full pinch.
    static constexpr auto fullRelease = 100.0f; // 100mms dist between fingers is considered a full release

    LEAP_VECTOR distVec = {thumbTip.x - fingerTip.x, thumbTip.y - fingerTip.y, thumbTip.z - fingerTip.z};
    return inverseLerp(fullRelease, fullPinch, vecLength(distVec));;
}

bool InteractionState::pinchStateChanged(bool &currentPinchState, float pinchValue) {
    if (!currentPinchState && pinchValue > triggerThreshold)
    {
        currentPinchState = true;
        return true;
    }
    else if (currentPinchState && pinchValue < releaseThreshold)
    {
        currentPinchState = false;
        return true;
    }
    return false;
}
