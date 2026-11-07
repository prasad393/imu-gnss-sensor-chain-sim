#pragma once

#include <chrono>
#include <optional>
#include <vector>

#include "component.h"
#include "messages.h"
#include "messaging.h"

// 50 Hz loop: mean of the new IMU samples and the newest GNSS position.
class Processing : public Component {
public:
    static constexpr milliseconds period{20};  // 1 / 50 Hz
    static constexpr double max_pos_age_s = 1.0;            // given by the task

    Processing(const Clock& clock, Topic<ImuSample>& imu, Topic<GnssSample>& gnss,
               Topic<Output>& out);

    // The rules without threads, so the test can call it.
    // A sample older than the previous tick is outdated.
    static Output filter(double t, double previous_tick, const std::vector<ImuSample>& imu,
                         const std::optional<GnssSample>& last_fix);

private:
    void step(double t) override;

    Pipe<ImuSample>& imu_;
    Pipe<GnssSample>& gnss_;
    Topic<Output>& out_;
    std::optional<GnssSample> last_fix_;
    double previous_tick_ = 0.0;
};
