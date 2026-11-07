#pragma once

#include <chrono>
#include <fstream>
#include <string>

#include "component.h"
#include "messages.h"
#include "messaging.h"

// Writes one CSV row per processing output, with the current alarms.
class Logger : public Component {
public:
    static constexpr milliseconds period{20};

    Logger(const Clock& clock, Topic<Output>& output, Topic<Alarms>& alarms,
           const std::string& path);

private:
    void step(double t) override;

    Pipe<Output>& output_;
    Pipe<Alarms>& alarms_;
    std::ofstream file_;
    Alarms state_{};
};
