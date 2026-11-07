#include "fdir.h"

#include <algorithm>
#include <iostream>

#include "processing.h"
#include "sensors.h"

namespace {
double to_seconds(milliseconds ms) { return seconds(ms).count(); }
}  // namespace

Fdir::Fdir(const Clock& clock, Topic<ImuSample>& imu, Topic<GnssSample>& gnss,
           Topic<Output>& output, Topic<Alarms>& alarms)
    : Component(clock, "fdir", period),
      imu_(imu.subscribe()),
      gnss_(gnss.subscribe()),
      output_(output.subscribe()),
      alarms_(alarms) {}

void Fdir::step(double t) {
    for (const ImuSample& s : imu_.drain()) {
        imu_seen_[s.id] = std::max(imu_seen_[s.id], s.t);
    }
    for (const GnssSample& s : gnss_.drain()) {
        gnss_seen_[s.id] = std::max(gnss_seen_[s.id], s.t);
    }
    for (const Output& o : output_.drain()) {
        output_seen_ = o.t;
        output_valid_ = o.rate_valid && o.pos_valid;
    }
    Alarms next{};
    next.t = t;
    // No valid output = a flag is 0, or no output for 3 periods.
    next.processing = !output_valid_ || missed(t, output_seen_, to_seconds(Processing::period));
    for (std::size_t i = 0; i < next.imu.size(); ++i) {
        next.imu[i] = missed(t, imu_seen_[i], to_seconds(ImuSensor::period));
    }
    for (std::size_t i = 0; i < next.gnss.size(); ++i) {
        next.gnss[i] = missed(t, gnss_seen_[i], to_seconds(GnssSensor::period));
    }

    const bool changed = next.processing != state_.processing || next.imu != state_.imu ||
                         next.gnss != state_.gnss;
    if (changed) {
        report(next);
        state_ = next;
        alarms_.publish(next);
    }
}

void Fdir::report(const Alarms& next) const {
    auto line = [&](const char* what, bool before, bool after) {
        if (before != after) {
            const char* status = after ? "ALARM" : "cleared";
            std::cout << "[" << next.t << "] "
                      << "fdir: " << what
                      << " " << status << '\n';
        }
    };
    line("processing", state_.processing, next.processing);
    line("imu1", state_.imu[0], next.imu[0]);
    line("imu2", state_.imu[1], next.imu[1]);
    line("imu3", state_.imu[2], next.imu[2]);
    line("gnss1", state_.gnss[0], next.gnss[0]);
    line("gnss2", state_.gnss[1], next.gnss[1]);
}
