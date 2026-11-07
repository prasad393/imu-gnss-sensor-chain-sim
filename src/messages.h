#pragma once

#include <array>

using Vec3 = std::array<double, 3>;

// Messages between components. Times are in seconds since the start.

struct ImuSample {
    int id;     // 0..2
    double t;
    Vec3 rate;  // rad/s, body axes
};

struct GnssSample {
    int id;     // 0..1
    double t;
    Vec3 pos;   // m
};

struct Output {
    double t;
    Vec3 rate;        // mean of the fresh IMU samples (NaN if none)
    bool rate_valid;
    int imu_samples;  // how many samples went into the mean
    Vec3 pos;         // last GNSS fix (NaN before the first one)
    bool pos_valid;
    double pos_age;   // s since the fix (since start if no fix yet)
};

struct Alarms {
    double t;
    bool processing;           // processing has no valid output
    std::array<bool, 3> imu;   // no sample for 3 intervals
    std::array<bool, 2> gnss;
};
