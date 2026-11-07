#include "logger.h"

#include <iomanip>
#include <stdexcept>

Logger::Logger(const Clock& clock, Topic<Output>& output, Topic<Alarms>& alarms,
               const std::string& path)
    : Component(clock, "logger", period),
      output_(output.subscribe()),
      alarms_(alarms.subscribe()),
      file_(path) {
    if (!file_) {
        throw std::runtime_error("cannot write " + path);
    }
    file_ << std::setprecision(10)
          << "t,rate_x,rate_y,rate_z,rate_valid,imu_samples,"
             "pos_x,pos_y,pos_z,pos_valid,pos_age,"
             "alarm_processing,alarm_imu1,alarm_imu2,alarm_imu3,alarm_gnss1,alarm_gnss2\n";
}

void Logger::step(double) {
    for (const Alarms& a : alarms_.drain()) {
        state_ = a;
    }
    for (const Output& o : output_.drain()) {
        file_ << o.t << ',' << o.rate[0] << ',' << o.rate[1] << ',' << o.rate[2] << ','
              << o.rate_valid << ',' << o.imu_samples << ',' << o.pos[0] << ',' << o.pos[1]
              << ',' << o.pos[2] << ',' << o.pos_valid << ',' << o.pos_age << ','
              << state_.processing << ',' << state_.imu[0] << ',' << state_.imu[1] << ','
              << state_.imu[2] << ',' << state_.gnss[0] << ',' << state_.gnss[1] << '\n';
    }
}
