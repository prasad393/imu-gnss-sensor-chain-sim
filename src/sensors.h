#pragma once

#include <atomic>
#include <chrono>
#include <random>
#include <string>
#include <thread>

#include "component.h"
#include "messages.h"
#include "messaging.h"

// none:   normal
// silent: runs but sends nothing
// slow:   each step takes 35 ms, so an IMU misses its 10 ms deadline
enum class Fault { none, silent, slow };

// Base of the IMU and GNSS sensors: fault injection and noise.
// The values come from a formula, not from real hardware.
class Sensor : public Component {
public:
    Sensor(const Clock& clock, std::string name, milliseconds period, int id);

    void set_fault(Fault fault) { fault_ = fault; }

protected:
    bool silent() const { return fault_ == Fault::silent; }
    void delay_if_slow() const {
        if (fault_ == Fault::slow) {
            std::this_thread::sleep_for(slow_delay);
        }
    }

    static constexpr milliseconds slow_delay{35};
    double noise(double sigma) { return std::normal_distribution<double>{0.0, sigma}(rng_); }

    const int id_;

private:
    std::atomic<Fault> fault_{Fault::none};
    std::mt19937 rng_;
};

// Attitude rate at 100 Hz: slow wobble + per-unit bias + noise.
class ImuSensor : public Sensor {
public:
    static constexpr milliseconds period{10};  // 1 / 100 Hz

    ImuSensor(const Clock& clock, int id, Topic<ImuSample>& out);

private:
    void step(double t) override;

    Topic<ImuSample>& out_;
};

// Position at 20 Hz: a circular orbit + noise.
class GnssSensor : public Sensor {
public:
    static constexpr milliseconds period{50};  // 1 / 20 Hz

    GnssSensor(const Clock& clock, int id, Topic<GnssSample>& out);

private:
    void step(double t) override;

    Topic<GnssSample>& out_;
};
