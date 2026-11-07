#include "sensors.h"

#include <cmath>

namespace {

constexpr double pi = 3.14159265358979323846;

// True values the sensors measure; they just need to change over the run.
Vec3 true_rate(double t) {
    return {0.02 * std::sin(2 * pi * 0.1 * t), 0.01 * std::cos(2 * pi * 0.05 * t), 0.005};
}

Vec3 true_position(double t) {
    constexpr double radius = 7.0e6;         // m
    constexpr double omega = 2 * pi / 5820;  // rad/s, ~97 min orbit
    return {radius * std::cos(omega * t), radius * std::sin(omega * t), 0.0};
}

}  // namespace

Sensor::Sensor(const Clock& clock, std::string name, milliseconds period, int id)
    : Component(clock, std::move(name), period), id_(id), rng_(static_cast<unsigned>(id)) {}

ImuSensor::ImuSensor(const Clock& clock, int id, Topic<ImuSample>& out)
    : Sensor(clock, "imu" + std::to_string(id + 1), period, id), out_(out) {}

void ImuSensor::step(double t) {
    if (silent()) {
        return;
    }
    delay_if_slow();  // the sample keeps its time t, so it arrives late
    const double bias = 5e-4 * id_;  // rad/s, so the 3 IMUs differ a little
    Vec3 rate = true_rate(t);
    for (double& axis : rate) {
        axis += bias + noise(1e-3);
    }
    out_.publish({id_, t, rate});
}

GnssSensor::GnssSensor(const Clock& clock, int id, Topic<GnssSample>& out)
    : Sensor(clock, "gnss" + std::to_string(id + 1), period, id), out_(out) {}

void GnssSensor::step(double t) {
    if (silent()) {
        return;
    }
    delay_if_slow();
    Vec3 pos = true_position(t);
    for (double& axis : pos) {
        axis += noise(5.0);
    }
    out_.publish({id_, t, pos});
}
