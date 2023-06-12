#pragma once
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <functional>
#include <vector>

#include "inclinometer.h"
#include "manager.h"
#include "util.h"
#include "wrappers_zephyr.h"

class InclinometerManager : public Manager {
public:
    InclinometerManager(Inclinometer& inclino);
    ~InclinometerManager() = default;
    int Init() override;
    void AddErrorCb(void (*cb)(void*), void* user_data) override;
    uint32_t GetLastXAngle();
    uint32_t GetLastYAngle();
    uint32_t GetLastZAngle();
    uint32_t GetLastValue();
    bool ChangeTimer(uint32_t new_time_ms);
    void StartInclinoTimer();
    void StopInclinoTimer();
    bool Subscribe(std::function<int(uint32_t)>);
    uint32_t GetSubscribeCount(void);

private:
    void ReadInclinoData();
    static void InclinoTimerHandler(struct k_timer* timer);
    static void ReadInclinoDataCallback(struct k_work* work);

private:
    Inclinometer& inclino_;
    double last_known_x_angle_;
    double last_known_y_angle_;
    double last_known_z_angle_;
    k_timer timer_;
    k_work_wrapper<InclinometerManager> work_wrapper_;
    CallbackWrapper on_error_;
    std::vector<std::function<int(uint32_t)>> subscribers_;
};