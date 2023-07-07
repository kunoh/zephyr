#pragma once
#include <functional>
#include <vector>

#include "inclinometer_manager.h"
#include "leg_control.h"
#include "manager.h"
#include "util.h"
#include "wrappers_zephyr.h"

class LegManager : public Manager {
public:
    LegManager(LegControl& leg_control, InclinometerManager& incl_mgr);
    ~LegManager() = default;
    int Init() override;
    int Selftest() override;
    void AddErrorCb(void (*cb)(void*), void* user_data) override;
    uint32_t GetLastValue();
    bool ChangeTimer(uint32_t new_time_ms);
    void StartLegTimer();
    void StopLegTimer();
    static int SubscribeCallback(SensorSampleData data);

private:
    void DoLegWork();
    static void LegTimerHandler(struct k_timer* timer);
    static void DoLegWorkCallback(struct k_work* work);

private:
    LegControl& leg_control_;
    InclinometerManager& incl_mgr_;
    k_timer timer_;
    k_work_wrapper<LegManager> work_wrapper_;
    uint32_t last_known_value_;
    CallbackWrapper on_error_;
};