#pragma once

#include <array>
#include <chrono>

#include "component.h"
#include "messages.h"
#include "messaging.h"

// Watches the sensors and the processing output; sends alarms when they change.
// Alarms are not latched: an alarm clears when its cause is gone.
class Fdir : public Component {
public:
    // As often as the fastest sensor (IMU).
    static constexpr milliseconds period{10};
    static constexpr int missed_intervals = 3;  // given by the task

    Fdir(const Clock& clock, Topic<ImuSample>& imu, Topic<GnssSample>& gnss,
         Topic<Output>& output, Topic<Alarms>& alarms);

    // True when no message came for more than missed_intervals periods.
    static bool missed(double now, double last_seen, double interval_s) {
        return now - last_seen > missed_intervals * interval_s;
    }

private:
    void step(double t) override;
    void report(const Alarms& next) const;

    Pipe<ImuSample>& imu_;
    Pipe<GnssSample>& gnss_;
    Pipe<Output>& output_;
    Topic<Alarms>& alarms_;

    std::array<double, 3> imu_seen_{};
    std::array<double, 2> gnss_seen_{};
    double output_seen_ = 0.0;
    bool output_valid_ = true;
    Alarms state_{};
};
