#pragma once

#include <zephyr/kernel.h>

#include <functional>
#include <memory>
#include <vector>

#include "battery.h"
#include "battery_charger.h"
#include "logger.h"
#include "util.h"

#define GENERAL_INIT_DELAY_LIMIT_MSEC 20
#define GENERAL_PERIOD_LOWER_LIMIT_MSEC 1000
#define GENERAL_INIT_DELAY_MSEC 20
#define GENERAL_PERIOD_MSEC 5000

#define CHARGING_INIT_DELAY_LOWER_LIMIT_MSEC 20
#define CHARGING_INIT_DELAY_UPPER_LIMIT_MSEC 120000
#define CHARGING_PERIOD_LOWER_LIMIT_MSEC 5000
#define CHARGING_PERIOD_UPPER_LIMIT_MSEC 60000
#define CHARGING_INIT_DELAY_MSEC 20
#define CHARGING_PERIOD_MSEC 10000

enum bat_data_t {
    GENERAL,
    CHARGING,
};

class BatteryManager {
public:
    BatteryManager(std::shared_ptr<Logger> logger, std::unique_ptr<Battery> battery,
                   std::unique_ptr<BatteryCharger> charger);
    ~BatteryManager() = default;

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
    void StartSampling(bat_data_t type, uint32_t init_delay_msec, uint32_t period_msec);
    void StopSampling(bat_data_t type);

    ///
    /// @brief Adds a function to a list of callback functions to call after each sampling of
    /// general battery properties.
    ///
    /// @param cb The callback function to add.
    ///
    void AddSubscriberGeneral(std::function<void(BatteryGeneralData)> cb);

    ///
    /// @brief Adds a function to a list of callback functions to call after each sampling of
    /// battery charging properties.
    ///
    /// @param cb The callback function to add.
    ///
    void AddSubscriberCharging(std::function<void(BatteryChargingData)> cb);

    size_t GetSubscriberCount(bat_data_t type);
    void ClearSubscribers(bat_data_t type);
    void SetCpuSubscribed(bool val);
    bool GetCpuSubscribed();

    int GetLastGeneralData(BatteryGeneralData& bat_gen_data);
    int GetLastChargingData(BatteryChargingData& bat_chg_data);

private:
    static void TimerQueueWork(struct k_timer* timer);
    static void HandleBatteryGeneralData(struct k_work* work);
    static void HandleBatteryChargingData(struct k_work* work);

    bool cpu_subscribed_ = false;
    BatteryGeneralData last_bat_gen_data_;
    BatteryChargingData last_bat_chg_data_;

    std::shared_ptr<Logger> logger_;
    std::unique_ptr<Battery> battery_;
    std::unique_ptr<BatteryCharger> charger_;
    std::pair<k_timer, k_work> timer_work_bat_gen_data_;
    std::pair<k_timer, k_work> timer_work_bat_chg_data_;
    std::vector<std::function<void(BatteryGeneralData)>> subscriber_cbs_gen_;
    std::vector<std::function<void(BatteryChargingData)>> subscriber_cbs_chg_;
};