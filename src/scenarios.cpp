#include "scenarios.h"

const std::vector<Scenario>& scenarios() {
    static const std::vector<Scenario> table = {
        {"nominal", 10.0, {}},

        // IMUs fail one by one; when the last one goes, the rate is flagged.
        {"imu_dropout",
         10.0,
         {{3.0, "imu1", Fault::silent},
          {5.0, "imu2", Fault::silent},
          {7.0, "imu3", Fault::silent}}},

        // Both GNSS silent for 0.5 s: FDIR alarms, the position stays valid.
        {"gnss_gap",
         10.0,
         {{4.0, "gnss1", Fault::silent},
          {4.0, "gnss2", Fault::silent},
          {4.5, "gnss1", Fault::none},
          {4.5, "gnss2", Fault::none}}},

        // IMU 1 slow for 2 s (35 ms per step): deadline missed, FDIR alarms.
        {"imu_slow",
         10.0,
         {{2.0, "imu1", Fault::slow},
          {4.0, "imu1", Fault::none}}},
    };
    return table;
}
