#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

#include "clock.h"

// Base of every task: its own thread calls step(t) once per period.
class Component {
public:
    Component(const Clock& clock, std::string name, milliseconds period);
    virtual ~Component();

    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    void start();
    void stop();  // returns once the thread has finished

    const std::string& name() const { return name_; }

protected:
    virtual void step(double t) = 0;

private:
    void run();

    const Clock& clock_;
    std::string name_;
    milliseconds period_;
    std::atomic<bool> stop_{false};
    bool late_ = false;  // last step missed its deadline
    std::thread thread_;
};
