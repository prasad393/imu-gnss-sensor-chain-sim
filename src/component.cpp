#include "component.h"

#include <iostream>
#include <utility>

Component::Component(const Clock& clock, std::string name, milliseconds period)
    : clock_(clock), name_(std::move(name)), period_(period) {}

Component::~Component() { stop(); }

void Component::start() { thread_ = std::thread(&Component::run, this); }

void Component::stop() {
    stop_ = true;
    if (thread_.joinable()) {
        thread_.join();
    }
}

void Component::run() {
    // Wake-up times are counted from the start, so the period does not drift.
    auto next = clock_.start() + period_;
    while (!stop_) {
        std::this_thread::sleep_until(next);
        step(clock_.now());
        const time_point done = steady_clock::now();

        // Deadline = the next start. Print only when this changes.
        const bool missed = done > next + period_;
        if (missed != late_) {
            const char* status = missed ? "deadline missed" : "back on time";
            std::cout << "[" << clock_.now() << "] "
                      << "task: " << name_
                      << " " << status << '\n';
            late_ = missed;
        }

        // Skip start times already in the past (no burst of late steps).
        do {
            next += period_;
        } while (next <= done);
    }
}
