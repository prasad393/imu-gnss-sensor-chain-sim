#pragma once

#include <chrono>

// Short names for the chrono types used in the project.
using milliseconds = std::chrono::milliseconds;      // whole milliseconds
using seconds      = std::chrono::duration<double>;  // seconds, with decimals
using steady_clock = std::chrono::steady_clock;      // a clock that never jumps back
using time_point   = steady_clock::time_point;       // a moment in time

// Seconds since the start of the run. All timestamps come from here.
class Clock {
public:
    Clock() {
        start_ = steady_clock::now();                 // remember when the run started
    }

    time_point start() const { return start_; }       // the start moment

    double now() const {                              // seconds since the start
        return seconds(steady_clock::now() - start_).count();
    }

private:
    time_point start_;
};
