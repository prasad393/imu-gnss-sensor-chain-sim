# Sensor chain simulation

Simulation of a small flight-software sensor chain, in C++17 with the standard library only.
3 IMUs and 2 GNSS receivers send data; Processing combines it every 20 ms; the Logger writes it to a CSV file.
FDIR raises an alarm when a sensor stops sending or the output is not valid. Faults are injected to test this.

## Design
docs/architecture.png is the design picture of the project.
Each box is one thread (main, 5 sensors, Processing, FDIR, Logger); each arrow is a message between them.

## Components
- IMU 1-3: 100 Hz (every 10 ms), sends attitude rate
- GNSS 1-2: 20 Hz (every 50 ms), sends position
- Processing: 50 Hz (every 20 ms), mean IMU rate + last GNSS position
- FDIR: every 10 ms, raises alarms
- Logger: every 20 ms, writes one CSV row per output

Components talk through topics: publish copies a message into one queue per reader.

## Rules
- Rate = mean of the IMU samples since the previous tick; older ones are outdated.
  No sample: `rate_valid = 0`.
- Position = newest GNSS fix. Older than 1 s: `pos_valid = 0`.
- FDIR alarm: sensor silent for 3 periods (IMU 30 ms, GNSS 150 ms),
  or Processing output invalid / missing.
- Deadline = period. A late step prints `deadline missed`; past start times are skipped.

## Build and run (Linux, g++ only)
In this folder (the one with `src/`):

    g++ -std=c++17 -O2 -pthread -Isrc src/*.cpp -o sim
    ./sim                 # all 4 runs, 10 s each
    ./sim imu_dropout     # one run

Test:

    g++ -std=c++17 -O2 -pthread -Isrc tests/test_rules.cpp src/component.cpp src/fdir.cpp src/logger.cpp src/processing.cpp src/scenarios.cpp src/sensors.cpp -o test_rules
    ./test_rules          # prints: test_rules: ok

## Runs
- nominal: no fault. Expected: only the start-up alarm (20-60 ms)
- imu_dropout: IMU 1, 2, 3 silent at 3, 5, 7 s. Expected: IMU alarms; after 7 s rate invalid + alarm
- gnss_gap: both GNSS silent 4.0-4.5 s. Expected: GNSS alarms; position stays valid (< 1 s)
- imu_slow (extra): IMU 1 step takes 35 ms from 2 to 4 s. Expected: `imu1 deadline missed`, imu1 alarm

## Output
- Console: events only.

      [3.000] scenario: imu1 -> silent
      [3.030] fdir: imu1 ALARM

- `log_<run>.csv`: one row every 20 ms: time, rate + `rate_valid`, `imu_samples`,
  position + `pos_valid`, `pos_age`, 6 alarms (1 = on).
- `logs/`: one real run of each scenario (4 CSV files + `console.txt`).

## Notes
- No GNSS fix before 50 ms, so `pos_valid = 0` and every run starts with an alarm at 20-60 ms
  (my choice: the task does not cover start-up).
- Alarms are live, not latched: they clear when the cause clears.
- FDIR every 10 ms and Logger every 20 ms are my choices.
- A desktop OS is not real time: a short pause can cause a brief alarm or deadline miss.

## Files
    src/clock.h            time since start
    src/component.*        periodic thread, base of all components
    src/messaging.h        Topic / Pipe
    src/sensors.*          IMU, GNSS, fault injection
    src/processing.*       50 Hz filter
    src/fdir.*             alarms
    src/logger.*           CSV log
    src/scenarios.*        the 4 runs
    src/main.cpp           wiring and fault playback
    tests/                 rule checks
    docs/architecture.png  design picture
