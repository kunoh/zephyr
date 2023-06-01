#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "inclinometer_manager.h"
#include "leg_control.h"
#include "logger.h"
#include "util.h"

class LegManager {
public:
    LegManager(std::shared_ptr<Logger> logger, LegControl* leg_control);
    ~LegManager() = default;
    uint32_t GetLastValue();
    bool ChangeTimer(uint32_t new_time_ms);
    void StartLegTimer();
    void StopLegTimer();
    bool InitSubscribtions(InclinometerManager* inclino_man);
    static int SubscribeCallback(uint32_t data);

private:
    static void LegTimerHandler(struct k_timer* timer);
    static void DoLegWork(struct k_work* work);

private:
    std::shared_ptr<Logger> logger_;
    LegControl* leg_control_;
    k_timer timer_;
    k_work work_;
    uint32_t last_known_value_;
};