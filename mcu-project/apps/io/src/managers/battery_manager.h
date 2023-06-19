#pragma once

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <functional>
#include <map>
#include <string>
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
#define CHARGING_REL_CHG_STATE_DEFAULT      CHARGING_REL_CHG_STATE_MAX

#define INSTALLATION_MODE_DEFAULT           installation_modes[0]
// clang-format on

enum bat_data_t {
    GENERAL,
    CHARGING,
    BAT_SENTINEL,
};

class BatteryManager : public Manager {
public:
    BatteryManager(Battery& battery, BatteryCharger& charger);
    ~BatteryManager() = default;
    int Init() override;
    void AddErrorCb(void (*cb)(void*), void* user_data) override;

    //--- Installation mode and limits ---//

    ///
    /// @brief Get the BatteryManager currently configured installation mode.
    ///
    std::string GetInstallationMode();

    /// @brief Set the BatteryManager currently configured installation mode.
    /// @param mode String specifying mode to set.
    /// @return 0 on success. EINVAL if the mode is not mapped.
    int SetInstallationMode(std::string mode);

    /// @brief Get the currently configured charging limit for a specific installation mode.
    /// @param mode Installation mode to get limit for.
    /// @param limit The configured limit.
    /// @return 0 on success. If the mode is not mapped.
    int GetModeChargingLimit(std::string mode, int32_t& limit);

    /// @brief Set the currently configured charging limit for a specific installation mode.
    /// @param mode Installation mode to get limit for.
    /// @param limit The limit to configure.
    /// @return 0 on success. If the mode is not mapped.
    int SetModeChargingLimit(std::string mode, int32_t limit);

    //--- Subscriptions ---///

    ///
    /// @brief Adds a function to a list of callback functions that will be called after each
    /// sampling of general battery properties.
    ///
    /// @param cb The callback function to add, taking a BatteryGeneralData struct.
    ///
    int AddSubscriberGeneral(std::function<int(BatteryGeneralData)> cb, std::string subscriber);

    ///
    /// @brief Adds a function to a list of callback functions that will be called after each
    /// sampling of battery charging properties.
    ///
    /// @param cb The callback function to add, taking a BatteryChargingData struct.
    ///
    int AddSubscriberCharging(std::function<int(BatteryChargingData)> cb, std::string subscriber);

    ///
    /// @brief Returns the number of subscribed callbacks for a specific battery data category.
    ///
    /// @param type Enum specifying type of subscribers.
    ///
    /// @return Number of subscribers for the specified type.
    ///
    size_t GetSubscriberCbCount(bat_data_t type);

    ///
    /// @brief Clear subscribers of a specific battery data category.
    ///
    /// @param type Enum specifying type to clear.
    ///
    /// @note Clearing GENERAL subscribers will also deassert the "cpu subscribed" status.
    ///
    void ClearSubscribers(bat_data_t type);

    ///
    /// @brief Check if a subscriber is subscribed to a specific type of battery data.
    ///
    bool IsSubscribed(std::string subscriber, bat_data_t data_type);

    //--- Sampling and charging ---//

    ///
    /// @brief Start sampling of a category of battery data.
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

    ///
    /// @brief Stop sampling of a category of battery data.
    ///
    void StopSampling(bat_data_t type);

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

    ///
    /// @brief Get manager's battery charging status.
    ///
    /// @return True if manager has written charging configuration to charger. False if charging has
    /// been inhibited.
    ///
    bool IsCharging();

private:
    void HandleBatteryGeneralData();
    void HandleBatteryChargingData();
    static void TimerQueueWork(struct k_timer* timer);
    static void HandleBatteryGeneralDataCallback(struct k_work* work);
    static void HandleBatteryChargingDataCallback(struct k_work* work);

    ///
    /// @brief Internal function for checking whether a certain pair of (subscriber type,  battery
    /// data type) exists as a key in the internal map.
    ///
    /// @return True if the key exists, false if not.
    ///
    bool SubscriptionTypeIsKnown(std::string sub, bat_data_t data_type);

    ///
    /// @brief Internal function for checking whether a mode string exists as a key in the internal
    /// map.
    ///
    /// @return True if the key exists, false if not.
    ///
    bool ModeIsRegistered(std::string mode);

    ///
    /// @brief Function for determining
    ///
    /// @return True if battery charging is allowed, false otherwise.
    ///
    bool ChargingAllowed();

    std::string installation_mode_ = INSTALLATION_MODE_DEFAULT;
    bool is_charging_ = false;
    std::map<std::pair<std::string, bat_data_t>, bool> subscriptions_;
    std::map<std::string, int32_t> chg_limits_;

    Battery& battery_;
    BatteryCharger& charger_;
    CallbackWrapper on_error_;
    std::pair<k_timer, k_work_wrapper<BatteryManager>> timer_work_bat_gen_data_;
    std::pair<k_timer, k_work_wrapper<BatteryManager>> timer_work_bat_chg_data_;
    std::vector<std::function<void(BatteryGeneralData)>> subscriber_cbs_gen_;
    std::vector<std::function<void(BatteryChargingData)>> subscriber_cbs_chg_;
    BatteryGeneralData last_bat_gen_data_;
    BatteryChargingData last_bat_chg_data_;
};