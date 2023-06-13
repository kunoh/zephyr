#pragma once

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <functional>
#include <map>
#include <vector>

#include "battery.h"
#include "battery_charger.h"
#include "manager.h"
#include "util.h"
#include "wrappers_zephyr.h"

// clang-format off
#define GENERAL_INIT_DELAY_MSEC             50
#define GENERAL_PERIOD_MSEC                 60000

#define CHARGING_INIT_DELAY_MSEC            50
// Sampling period for charging data needs to be less than 3.5 s to reset charger WDT.
#define CHARGING_PERIOD_UPPER_LIMIT_MSEC    3000
#define CHARGING_PERIOD_MSEC                CHARGING_PERIOD_UPPER_LIMIT_MSEC

#define CHARGING_REL_CHG_STATE_MIN          20
#define CHARGING_REL_CHG_STATE_MAX          100
#define CHARGING_REL_CHG_STATE_DEFAULT      80
// clang-format on

enum bat_data_t {
    GENERAL,
    CHARGING,
};

enum installation_mode_t {
    MOBILE,
    FIXED,
    SENTINEL,
};

class BatteryManager : public Manager {
public:
    BatteryManager(Battery& battery, BatteryCharger& charger);
    ~BatteryManager() = default;
    int Init() override;
    void AddErrorCb(void (*cb)(void*), void* user_data) override;

    ///
    /// @brief Start battery sampling.
    ///
    /// \note It is permitted to call StartSampling() while already started.
    ///       This will restart the sampling timers with the new specified parameters.
    /// @param[in] type The category of battery properties to start sampling. Can be general or
    /// charging related.
    /// @param[in] init_delay_msec The initial delay (in milliseconds) from calling sampling start
    /// to the first sample is fetched.
    /// @param[in] period_msec The sampling period (in milliseconds).
    ///
    int StartSampling(bat_data_t type, uint32_t init_delay_msec, uint32_t period_msec);

    void StopSampling(bat_data_t type);

    ///
    /// @brief Adds a function to a list of callback functions that will be called after each
    /// sampling of general battery properties.
    ///
    /// @param cb The callback function to add, taking a BatteryGeneralData struct.
    ///
    void AddSubscriberGeneral(std::function<void(BatteryGeneralData)> cb);

    ///
    /// @brief Adds a function to a list of callback functions that will be called after each
    /// sampling of battery charging properties.
    ///
    /// @param cb The callback function to add, taking a BatteryChargingData struct.
    ///
    void AddSubscriberCharging(std::function<void(BatteryChargingData)> cb);

    ///
    /// @brief Returns the number of subscribed callbacks for a specific battery data category.
    ///
    /// @param type Enum specifying type of subscribers.
    ///
    /// @return Number of subscribers for the specified type.
    ///
    size_t GetSubscriberCount(bat_data_t type);

    ///
    /// @brief Clear subscribers of a specific battery data category.
    ///
    /// @param type Enum specifying type to clear.
    ///
    /// @note Clearing GENERAL subscribers will also deassert the "cpu subscribed" status.
    ///
    void ClearSubscribers(bat_data_t type);

    ///
    /// @brief Function to specificy cpu subscribed status. Should be used in conjunction with a
    /// AddSubscriber<x>() call.
    ///
    void SetCpuSubscribed(bool val);

    ///
    /// @brief Get whether CPU is considered subscribed to battery notifications or not.
    ///
    /// @return True if CPU is subscribed, false otherwise.
    ///
    bool CpuIsSubscribed();

    ///
    /// @brief Get manager's battery charging status.
    ///
    /// @return True if manager has written charging configuration to charger. False if charging has
    /// been inhibited.
    ///
    bool IsCharging();
    bool ModeIsKnown(int32_t mode);
    void SetInstallationMode(installation_mode_t mode);
    installation_mode_t GetInstallationMode();
    int SetModeChargingLimit(installation_mode_t mode, int32_t limit);
    int GetModeChargingLimit(installation_mode_t mode, int32_t& limit);

    ///
    /// @brief Get last general battery data sampling manager is holding.
    ///
    /// @param[out] bat_gen_data Struct to populate with manager last known values. Will be
    /// populated with invalid default values if sampling timer is not running.
    ///
    /// @return 0 on success. EIO if sampling timer is not running at moment of call.
    ///
    int GetLastGeneralData(BatteryGeneralData& bat_gen_data);

    ///
    /// @brief Get last charging battery data sampling manager is holding.
    ///
    /// @param[out] bat_gen_data Struct to populate with manager last known values. Will be
    /// populated with invalid default values if sampling timer is not running.
    ///
    /// @return 0 on success. EIO if sampling timer is not running at moment of call.
    ///
    int GetLastChargingData(BatteryChargingData& bat_chg_data);

private:
    void HandleBatteryGeneralData();
    void HandleBatteryChargingData();
    static void TimerQueueWork(struct k_timer* timer);
    static void HandleBatteryGeneralDataCallback(struct k_work* work);
    static void HandleBatteryChargingDataCallback(struct k_work* work);
    bool ChargingAllowed();

    bool is_charging_ = false;
    bool cpu_subscribed_ = false;
    BatteryGeneralData last_bat_gen_data_;
    BatteryChargingData last_bat_chg_data_;
    std::map<installation_mode_t, int32_t> chg_limits_;
    installation_mode_t installation_mode_ = MOBILE;

    Battery& battery_;
    BatteryCharger& charger_;
    CallbackWrapper on_error_;
    std::pair<k_timer, k_work_wrapper<BatteryManager>> timer_work_bat_gen_data_;
    std::pair<k_timer, k_work_wrapper<BatteryManager>> timer_work_bat_chg_data_;
    std::vector<std::function<void(BatteryGeneralData)>> subscriber_cbs_gen_;
    std::vector<std::function<void(BatteryChargingData)>> subscriber_cbs_chg_;
};