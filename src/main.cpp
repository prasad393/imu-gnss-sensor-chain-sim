#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "clock.h"
#include "fdir.h"
#include "logger.h"
#include "processing.h"
#include "scenarios.h"
#include "sensors.h"

namespace {

const char* fault_name(Fault fault) {
    switch (fault) {
        case Fault::silent: return "silent";
        case Fault::slow:   return "slow";
        case Fault::none:   break;
    }
    return "nominal";
}

// The whole chain: makes all components, runs one scenario, then stops them.
class Simulation {
public:
    explicit Simulation(const std::string& log_path) {
        for (int id = 0; id < 3; ++id) {
            add_sensor(std::make_unique<ImuSensor>(clock_, id, imu_));
        }
        for (int id = 0; id < 2; ++id) {
            add_sensor(std::make_unique<GnssSensor>(clock_, id, gnss_));
        }
        components_.push_back(std::make_unique<Processing>(clock_, imu_, gnss_, output_));
        components_.push_back(std::make_unique<Fdir>(clock_, imu_, gnss_, output_, alarms_));
        components_.push_back(std::make_unique<Logger>(clock_, output_, alarms_, log_path));
    }

    ~Simulation() {
        // Stop readers first, so FDIR does not report the sensors stopping.
        for (auto it = components_.rbegin(); it != components_.rend(); ++it) {
            (*it)->stop();
        }
    }

    void run(const Scenario& scenario) {
        for (auto& component : components_) {
            component->start();
        }
        // Apply each fault at its time, then wait for the end of the run.
        for (const FaultEvent& event : scenario.events) {
            std::this_thread::sleep_until(clock_.start() + seconds(event.t));
            sensor(event.sensor).set_fault(event.fault);
            std::cout << "[" << clock_.now() << "] "
                      << "scenario: " << event.sensor
                      << " -> " << fault_name(event.fault) << '\n';
        }
        std::this_thread::sleep_until(clock_.start() +
                                      seconds(scenario.duration_s));
    }

private:
    void add_sensor(std::unique_ptr<Sensor> sensor) {
        sensors_.push_back(sensor.get());
        components_.push_back(std::move(sensor));
    }

    Sensor& sensor(const std::string& name) {
        for (Sensor* s : sensors_) {
            if (s->name() == name) {
                return *s;
            }
        }
        throw std::runtime_error("unknown sensor: " + name);
    }

    Clock clock_;
    Topic<ImuSample> imu_;
    Topic<GnssSample> gnss_;
    Topic<Output> output_;
    Topic<Alarms> alarms_;
    std::vector<std::unique_ptr<Component>> components_;
    std::vector<Sensor*> sensors_;
};

void run_scenario(const Scenario& scenario) {
    const std::string log = "log_" + scenario.name + ".csv";
    std::cout << "=== " << scenario.name << ": "
              << static_cast<int>(scenario.duration_s) << " s, log in " << log << '\n';
    Simulation sim(log);
    sim.run(scenario);
}

}  // namespace

int main(int argc, char** argv) {
    std::cout << std::fixed << std::setprecision(3);  // times in seconds, like 2.045

    const std::string pick = argc > 1 ? argv[1] : "all";

    bool found = false;
    for (const Scenario& scenario : scenarios()) {
        if (pick == "all" || pick == scenario.name) {
            run_scenario(scenario);
            found = true;
        }
    }
    if (!found) {
        std::cerr << "usage: " << argv[0] << " [all";
        for (const Scenario& scenario : scenarios()) {
            std::cerr << "|" << scenario.name;
        }
        std::cerr << "]\n";
        return 2;
    }
    return 0;
}
