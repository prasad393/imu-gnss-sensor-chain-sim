#pragma once

#include <string>
#include <vector>

#include "sensors.h"

// A run: its length and a list of faults (time, sensor, fault).

struct FaultEvent {
    double t;
    std::string sensor;  // component name, e.g. "imu2"
    Fault fault;
};

struct Scenario {
    std::string name;
    double duration_s;
    std::vector<FaultEvent> events;  // in time order
};

const std::vector<Scenario>& scenarios();
