// Checks the processing and FDIR rules. No threads, no test framework.

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "fdir.h"
#include "processing.h"

#define CHECK(cond)                                                                    \
    do {                                                                               \
        if (!(cond)) {                                                                 \
            std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK(" #cond ") failed\n"; \
            std::exit(1);                                                              \
        }                                                                              \
    } while (0)

int main() {
    // Mean of the samples since the previous tick (0.98); 0.90 is outdated.
    {
        const std::vector<ImuSample> imu = {
            {0, 0.99, {1.0, 2.0, 3.0}}, {1, 1.00, {3.0, 4.0, 5.0}}, {2, 0.90, {9.0, 9.0, 9.0}}};
        const Output out = Processing::filter(1.0, 0.98, imu, std::nullopt);
        CHECK(out.rate_valid);
        CHECK(out.imu_samples == 2);
        CHECK(out.rate[0] == 2.0 && out.rate[1] == 3.0 && out.rate[2] == 4.0);
        CHECK(!out.pos_valid && std::isnan(out.pos[0]));
    }

    // No fresh sample: flag raised, rate is NaN.
    {
        const Output out = Processing::filter(1.0, 0.98, {{0, 0.5, {1.0, 1.0, 1.0}}}, std::nullopt);
        CHECK(!out.rate_valid && out.imu_samples == 0 && std::isnan(out.rate[0]));
    }

    // Position: last fix is kept; flag raised when older than 1 s.
    {
        const GnssSample fix{0, 2.0, {1.0, 2.0, 3.0}};
        CHECK(Processing::filter(3.0, 2.98, {}, fix).pos_valid);
        const Output out = Processing::filter(3.05, 3.03, {}, fix);
        CHECK(!out.pos_valid && out.pos[0] == 1.0);
    }

    // Alarm after 3 missed intervals: 30 ms for an IMU, 150 ms for a GNSS.
    CHECK(!Fdir::missed(1.029, 1.0, 0.01));
    CHECK(Fdir::missed(1.031, 1.0, 0.01));
    CHECK(!Fdir::missed(1.149, 1.0, 0.05));
    CHECK(Fdir::missed(1.151, 1.0, 0.05));

    std::cout << "test_rules: ok\n";
    return 0;
}
