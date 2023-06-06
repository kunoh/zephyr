#pragma once
#include <zephyr/kernel.h>

#include <functional>
#include <memory>
#include <vector>

#include "inclinometer.h"
#include "logger.h"
#include "util.h"

class InclinometerManager {
public:
    InclinometerManager(std::shared_ptr<Logger> logger, std::unique_ptr<Inclinometer> inclino);
    ~InclinometerManager() = default;
    uint32_t GetLastXAngle();
    uint32_t GetLastYAngle();
    uint32_t GetLastZAngle();
    bool ChangeTimer(uint32_t new_time_ms);
    void StartInclinoTimer();
    void StopInclinoTimer();
    bool Subscribe(std::function<int(uint32_t)>);
    uint32_t GetSubscribeCount(void);

private:
    static void InclinoTimerHandler(struct k_timer *timer);
    static void ReadInclinoData(struct k_work *work);

private:
    std::shared_ptr<Logger> logger_;
    std::unique_ptr<Inclinometer> inclino_;
    k_timer timer_;
    k_work work_;
    double last_known_x_angle_;
    double last_known_y_angle_;
    double last_known_z_angle_;
    std::vector<std::function<int(uint32_t)>> subscribers_;
};