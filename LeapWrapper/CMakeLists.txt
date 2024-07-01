add_library(leap_tracker STATIC
        LeapTracker.h
        LeapTracker.cpp
)
target_compile_features(leap_tracker PUBLIC cxx_std_17)
target_link_libraries(leap_tracker PUBLIC LeapSDK::LeapC)