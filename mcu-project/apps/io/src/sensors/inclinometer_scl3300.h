#pragma once
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/util.h>

#include <cstdlib>

#include "inclinometer.h"
#include "logger.h"

class InclinometerScl3300 : public Inclinometer {
public:
    InclinometerScl3300(Logger& logger);
    virtual ~InclinometerScl3300() = default;
    int Init() override;
    bool Write() override;
    bool Read() override;
    void GetAngle(double xyz_angle[]) override;

private:
    Logger& logger_;
    const struct device* spi_dev_ = DEVICE_DT_GET(DT_NODELABEL(lpspi1));
    const struct device* incl_dev_ = DEVICE_DT_GET(DT_NODELABEL(incl_scl3300));
    double acc_x_;
    double acc_y_;
    double acc_z_;
    double angle_x_;
    double angle_y_;
    double angle_z_;
};