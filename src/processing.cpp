#include "processing.h"

#include <limits>

Processing::Processing(const Clock& clock, Topic<ImuSample>& imu, Topic<GnssSample>& gnss,
                       Topic<Output>& out)
    : Component(clock, "processing", period),
      imu_(imu.subscribe()),
      gnss_(gnss.subscribe()),
      out_(out) {}

void Processing::step(double t) {
    // Newest fix of the two GNSS units.
    for (const GnssSample& fix : gnss_.drain()) {
        if (!last_fix_ || fix.t > last_fix_->t) {
            last_fix_ = fix;
        }
    }
    out_.publish(filter(t, previous_tick_, imu_.drain(), last_fix_));
    previous_tick_ = t;
}

Output Processing::filter(double t, double previous_tick, const std::vector<ImuSample>& imu,
                          const std::optional<GnssSample>& last_fix) {
    constexpr double nan = std::numeric_limits<double>::quiet_NaN();

    Output out{};
    out.t = t;

    out.rate = {0.0, 0.0, 0.0};
    for (const ImuSample& sample : imu) {
        if (sample.t < previous_tick) {
            continue;  // outdated
        }
        for (int i = 0; i < 3; ++i) {
            out.rate[i] += sample.rate[i];
        }
        ++out.imu_samples;
    }
    out.rate_valid = out.imu_samples > 0;
    for (double& axis : out.rate) {
        axis = out.rate_valid ? axis / out.imu_samples : nan;
    }

    if (last_fix) {
        out.pos = last_fix->pos;
        out.pos_age = t - last_fix->t;
        out.pos_valid = out.pos_age <= max_pos_age_s;
    } else {
        out.pos = {nan, nan, nan};
        // No GNSS fix yet (the first comes at 50 ms).
        // The task does not cover this; I raise the flag.
        out.pos_age = t;  // nothing received since start
        out.pos_valid = false;
    }
    return out;
}
