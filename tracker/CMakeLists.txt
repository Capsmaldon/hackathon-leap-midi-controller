add_library(leap_tracker STATIC
        tracker.h
        tracker.cpp
)

target_link_libraries(leap_tracker PUBLIC LeapSDK::LeapC)