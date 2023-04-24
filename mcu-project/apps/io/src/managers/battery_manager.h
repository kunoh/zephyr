#pragma once

#include <zephyr/kernel.h>

#include <functional>
#include <memory>
#include <vector>

#include "battery.h"
#include "logger.h"
#include "util.h"

#define SAMPLE_TRIGGER_INIT_DELAY_LIMIT_MSEC 1
#define SAMPLE_TRIGGER_PERIOD_LIMIT_MSEC 500
#define SAMPLE_TRIGGER_INIT_DELAY_MSEC 50
#define SAMPLE_TRIGGER_PERIOD_MSEC 5000

#define TIMER_DEFAULT_INIT_DELAY_MSEC SAMPLE_TRIGGER_INIT_DELAY_MSEC
#define TIMER_DEFAULT_PERIOD_MSEC SAMPLE_TRIGGER_PERIOD_MSEC

class BatteryManager {
public:
    BatteryManager(std::shared_ptr<Logger> logger, std::unique_ptr<Battery> battery);
    ~BatteryManager() = default;

    ///
    /// @brief Adds a function to a list of callback functions to call after each battery sampling.
    ///
    /// @param cb The callback function to add.
    ///
    void AddSubscriber(std::function<void(BatteryData)> cb);
    size_t GetSubscriberCount();
    void ClearSubscribers();

    ///
    /// @brief Start battery sampling.
    ///
    /// \note It is permitted to call StartSampling() while already started.
    ///       This will restart the sampling timers with the new specified parameters.
    ///
    /// @param[in] init_delay_msec The initial delay (in milliseconds) from calling sampling start
    /// to the first sample is fetched.
    /// @param[in] period_msec The sampling period (in milliseconds).
    ///
    void StartSampling(uint32_t init_delay_msec = TIMER_DEFAULT_INIT_DELAY_MSEC,
                       uint32_t period_msec = TIMER_DEFAULT_PERIOD_MSEC);

    void StopSampling();
    void SetCpuSubscribed(bool val);
    bool GetCpuSubscribed();

    int TriggerAndGetSample(BatteryData* battery_sample_data);

private:
    static void TimerQueueWork(struct k_timer* timer);
    static void NotifySubscribers(struct k_work* work);

    bool cpu_subscribed_ = false;
    std::shared_ptr<Logger> logger_;
    std::unique_ptr<Battery> battery_;
    std::pair<k_timer, k_work> timer_work_sample_get_data_;
    std::vector<std::function<void(BatteryData)> > subscriber_cbs_;
};