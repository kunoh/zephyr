#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "inclinometer_manager.h"
#include "leg_control.h"
#include "logger.h"
#include "manager.h"
#include "util.h"
#include "wrappers_zephyr.h"

class LegManager : public Manager {
public:
    LegManager(Logger& logger, LegControl& leg_control, InclinometerManager& incl_mgr);
    ~LegManager() = default;
    int Init() override;
    void AddErrorCb(void (*cb)(void*), void* user_data) override;
    uint32_t GetLastValue();
    bool ChangeTimer(uint32_t new_time_ms);
    void StartLegTimer();
    void StopLegTimer();
    static int SubscribeCallback(uint32_t data);

private:
    void DoLegWork();
    static void LegTimerHandler(struct k_timer* timer);
    static void DoLegWorkCallback(struct k_work* work);

private:
    Logger& logger_;
    LegControl& leg_control_;
    InclinometerManager& incl_mgr_;
    k_timer timer_;
    k_work_wrapper<LegManager> work_wrapper_;
    uint32_t last_known_value_;
    CallbackWrapper on_error_;
};