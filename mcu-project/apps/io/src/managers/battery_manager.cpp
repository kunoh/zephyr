#include "battery_manager.h"

#include <zephyr/logging/log.h>

#include <cmath>

LOG_MODULE_REGISTER(bat_mgr, CONFIG_BATTERY_MANAGER_LOG_LEVEL);

BatteryManager::BatteryManager(Battery& battery, BatteryCharger& charger)
    : battery_{battery}, charger_{charger}
{
    // Periodic charging information
    k_timer_init(&timer_work_bat_chg_data_.first, TimerQueueWork, NULL);
    k_timer_user_data_set(&timer_work_bat_chg_data_.first, &timer_work_bat_chg_data_.second.work);
    k_work_init(&timer_work_bat_chg_data_.second.work,
                &BatteryManager::HandleBatteryChargingDataCallback);
    timer_work_bat_chg_data_.second.self = this;

    // Periodic general information
    k_timer_init(&timer_work_bat_gen_data_.first, TimerQueueWork, NULL);
    k_timer_user_data_set(&timer_work_bat_gen_data_.first, &timer_work_bat_gen_data_.second.work);
    k_work_init(&timer_work_bat_gen_data_.second.work,
                &BatteryManager::HandleBatteryGeneralDataCallback);
    timer_work_bat_gen_data_.second.self = this;
}

int BatteryManager::Init()
{
    int ret = 0;

    // Initialize installation mode charging limits
    for (int32_t mode_num = 0; mode_num < NUM_INSTALLATION_MODES; mode_num++) {
        chg_limits_.emplace(installation_modes[mode_num], CHARGING_REL_CHG_STATE_DEFAULT);
    }

    // Initialize battery data subscription mapping
    GeneralSubscription gen_sub;
    ChargingSubscription chg_sub;
    for (int32_t sub_num = 0; sub_num < NUM_SUBSCRIBER_TYPES; sub_num++) {
        subscriptions_gen_.emplace(subscriber_types[sub_num], gen_sub);
        subscriptions_chg_.emplace(subscriber_types[sub_num], chg_sub);
    }

    ret = battery_.Init();
    if (ret != 0) {
        LOG_ERR("Failed to initialize battery.");
        return ret;
    }
    LOG_INF("Battery initialized.");
    StartSampling("GENERAL", GENERAL_INIT_DELAY_MSEC, GENERAL_PERIOD_MSEC);

    ret = charger_.Init();
    if (ret != 0) {
        LOG_ERR("Failed to initialize charger.");
        return ret;
    }
    LOG_INF("Charger initialized.");
    StartSampling("CHARGING", CHARGING_INIT_DELAY_MSEC, CHARGING_PERIOD_MSEC);

    LOG_INF("Battery manager initialized.");
    return 0;
}

void BatteryManager::AddErrorCb(void (*cb)(void*), void* user_data)
{
    on_error_.cb = cb;
    on_error_.user_data = user_data;
}

std::string BatteryManager::GetInstallationMode() const
{
    return installation_mode_;
}

int BatteryManager::SetInstallationMode(const std::string& mode)
{
    if (!ModeIsRegistered(mode)) {
        return EINVAL;
    }

    installation_mode_ = mode;
    return 0;
}

int BatteryManager::GetModeChargingLimit(const std::string& mode, int32_t& limit)
{
    if (!ModeIsRegistered(mode)) {
        return EINVAL;
    }
    limit = chg_limits_[mode];
    return 0;
}

int BatteryManager::SetModeChargingLimit(const std::string& mode, int32_t limit)
{
    if (!ModeIsRegistered(mode)) {
        return EINVAL;
    } else if (limit < CHARGING_REL_CHG_STATE_MIN || limit > CHARGING_REL_CHG_STATE_MAX) {
        return ERANGE;
    }

    chg_limits_[mode] = limit;
    return 0;
}

int BatteryManager::AddSubscriber(const std::string& subscriber, const std::string& data_type,
                                  std::function<int(BatteryGeneralData)> cb)
{
    if (!SubscriptionTypeIsKnown(subscriber, data_type)) {
        LOG_ERR("Subscription type (%s, %s) is unknown.", subscriber.c_str(), data_type.c_str());
        return EINVAL;
    } else if (IsSubscribed(subscriber, data_type)) {
        LOG_INF("Subscription type (%s, %s) already exists.", subscriber.c_str(),
                data_type.c_str());
        return EEXIST;
    }

    LOG_INF("Added %s subcription to general data.", subscriber.c_str());
    subscriptions_gen_[subscriber].cb = cb;
    return 0;
}

int BatteryManager::AddSubscriber(const std::string& subscriber, const std::string& data_type,
                                  std::function<int(BatteryChargingData)> cb)
{
    if (!SubscriptionTypeIsKnown(subscriber, data_type)) {
        LOG_ERR("Subscription type (%s, %s) is unknown.", subscriber.c_str(), data_type.c_str());
        return EINVAL;
    } else if (IsSubscribed(subscriber, data_type)) {
        LOG_INF("Subscription type (%s, %s) already exists.", subscriber.c_str(),
                data_type.c_str());
        return EEXIST;
    }

    LOG_INF("Added %s subcription to charging data.", subscriber.c_str());
    subscriptions_chg_[subscriber].cb = cb;
    return 0;
}

void BatteryManager::ClearSubscribers(const std::string& data_type)
{
    if (data_type == "GENERAL") {
        for (auto& pair : subscriptions_gen_) {
            pair.second.cb = NULL;
        }
        LOG_INF("Cleared GENERAL subscribers.");
    } else if (data_type == "CHARGING") {
        for (auto& pair : subscriptions_chg_) {
            pair.second.cb = NULL;
        }
        LOG_INF("Cleared CHARGING subscribers.");
    } else {
        LOG_ERR("Trying to clear non-existing type of subscribers.");
    }
}

bool BatteryManager::IsSubscribed(const std::string& subscriber, const std::string& data_type)
{
    if (!SubscriptionTypeIsKnown(subscriber, data_type)) {
        return false;
    }

    if (data_type == "GENERAL") {
        if (subscriptions_gen_[subscriber].cb != NULL) {
            return true;
        } else {
            return false;
        }
    } else if (data_type == "CHARGING") {
        if (subscriptions_chg_[subscriber].cb != NULL) {
            return true;
        } else {
            return false;
        }
    }

    LOG_WRN("Failed to get subscription type (%s, %s)", subscriber.c_str(), data_type.c_str());
    return false;
}

int BatteryManager::SetNotificationThresholds(const std::string& subscriber,
                                              const BatteryGeneralData& thresholds)
{
    if (!SubscriptionTypeIsKnown(subscriber, "GENERAL")) {
        return EINVAL;
    }

    subscriptions_gen_[subscriber].thresholds = thresholds;
    return 0;
}

int BatteryManager::SetNotificationThresholds(const std::string& subscriber,
                                              const BatteryChargingData& thresholds)
{
    if (!SubscriptionTypeIsKnown(subscriber, "CHARGING")) {
        return EINVAL;
    }

    subscriptions_chg_[subscriber].thresholds = thresholds;
    return 0;
}

int BatteryManager::StartSampling(const std::string& data_type, const uint32_t init_delay_msec,
                                  const uint32_t period_msec)
{
    int ret = 0;
    if (data_type == "GENERAL") {
        k_timer_start(&timer_work_bat_gen_data_.first, K_MSEC(init_delay_msec),
                      K_MSEC(period_msec));

    } else if (data_type == "CHARGING") {
        if (period_msec > CHARGING_PERIOD_UPPER_LIMIT_MSEC) {
            LOG_ERR("Tried to set too slow battery data charging sample rate.");
            return ERANGE;
        }

        k_timer_start(&timer_work_bat_chg_data_.first, K_MSEC(init_delay_msec),
                      K_MSEC(period_msec));

    } else {
        LOG_ERR("Trying to start non-existing bat sampling timer.");
        ret = EINVAL;
    }

    return ret;
}

void BatteryManager::StopSampling(const std::string& data_type)
{
    if (data_type == "GENERAL") {
        k_timer_stop(&timer_work_bat_gen_data_.first);
    } else if (data_type == "CHARGING") {
        k_timer_stop(&timer_work_bat_chg_data_.first);
    } else {
        LOG_ERR("Trying to stop non-existing bat sampling timer.");
    }
}

int BatteryManager::GetLastData(BatteryGeneralData& bat_data)
{
    // If sampling timer is stopped, data might be old.
    if (k_timer_remaining_ticks(&timer_work_bat_gen_data_.first) != 0) {
        bat_data.temp = last_gen_data_.temp;
        bat_data.current = last_gen_data_.current;
        bat_data.volt = last_gen_data_.volt;
        bat_data.remaining_capacity = last_gen_data_.remaining_capacity;
        bat_data.cycle_count = last_gen_data_.cycle_count;
    } else {
        bat_data.temp = DEFAULT_INVALID_BAT_FLOAT;
        bat_data.current = DEFAULT_INVALID_BAT_FLOAT;
        bat_data.volt = DEFAULT_INVALID_BAT_FLOAT;
        bat_data.remaining_capacity = DEFAULT_INVALID_BAT_INT;
        bat_data.cycle_count = DEFAULT_INVALID_BAT_INT;
        return EIO;
    }

    return 0;
}

int BatteryManager::GetLastData(BatteryChargingData& bat_data)
{
    // If sampling timer is stopped, data might be old.
    if (k_timer_remaining_ticks(&timer_work_bat_chg_data_.first) != 0) {
        bat_data.des_chg_current = last_chg_data_.des_chg_current;
        bat_data.des_chg_volt = last_chg_data_.des_chg_volt;
        bat_data.status = last_chg_data_.status;
        bat_data.relative_charge_state = last_chg_data_.relative_charge_state;
        bat_data.charging = is_charging_;
    } else {
        bat_data.des_chg_current = DEFAULT_INVALID_BAT_INT;
        bat_data.des_chg_volt = DEFAULT_INVALID_BAT_INT;
        bat_data.status = DEFAULT_INVALID_BAT_INT;
        bat_data.relative_charge_state = DEFAULT_INVALID_BAT_INT;
        bat_data.charging = false;
        return EIO;
    }

    return 0;
}

bool BatteryManager::IsCharging() const
{
    return is_charging_;
}

//--- Private member functions ---//

void BatteryManager::HandleBatteryGeneralData()
{
    if (battery_.TriggerGeneralSampling() != 0) {
        LOG_ERR("Failed sampling one or more general battery properties.");
    }
    if (battery_.GetGeneralData(last_gen_data_) != 0) {
        LOG_ERR("Failed fetching one or more general battery properties.");
    }

    for (auto& pair : subscriptions_gen_) {
        if (pair.second.cb != NULL && IsOverThreshold(pair.second, last_gen_data_)) {
            pair.second.cb(last_gen_data_);
            pair.second.last_notificaton_ = last_gen_data_;
        }
    }
}

void BatteryManager::HandleBatteryChargingData()
{
    bool inhibit_charging = false;
    int ret = 0;
    // Get charging info from battery and determine whether or not it's in a condition to be
    // charged. If we cannot sample battery reliably or battery alarm flags are set, charging is
    // inhibited.
    if ((battery_.TriggerChargingSampling() != 0) ||
        (battery_.GetChargingData(last_chg_data_) != 0)) {
        LOG_WRN("Failed sampling one or more battery charging properties.");
        inhibit_charging = true;
    } else if (!ChargingAllowed()) {
        if (is_charging_) {
            LOG_INF("Battery cannot be charged. Battery status: 0x%X", last_chg_data_.status);
        }
        inhibit_charging = true;
    }

    if (inhibit_charging) {
        ret = charger_.SetChargingCurrent(0);
        if (ret != 0) {
            LOG_ERR("Couldn't inhibit charging!");
        } else if (is_charging_) {
            LOG_INF("Inhibited charging.");
            is_charging_ = false;
        }
    } else {
        if (!is_charging_) {
            LOG_INF("Configuring battery charging.");
        }
        // Attempt to configure charge controller with charging properties from battery.
        ret =
            charger_.SetChargingConfig(last_chg_data_.des_chg_current, last_chg_data_.des_chg_volt);

        if (ret != 0) {
            switch (ret) {
                case ERRNO_CHARGER_VOLTAGE_EIO:
                    LOG_ERR("An IO-error occured while attempting to set max charging voltage.");
                    break;
                case ERRNO_CHARGER_VOLTAGE_ERANGE:
                    LOG_ERR(
                        "Attempting to set max charging voltage outside operable charger range.");
                    break;
                case ERRNO_CHARGER_CURRENT_EIO:
                    LOG_ERR("An IO-error occured while attempting to set max charging current.");
                    break;
                case ERRNO_CHARGER_CURRENT_ERANGE:
                    LOG_ERR("Attempting to set charging current outside operable charger range.");
                    break;
                default:
                    LOG_ERR("SetChargingConfig() returned non-zero errno.");
                    break;
            }
        } else {
            is_charging_ = true;
        }
    }

    // Send to subscribers
    last_chg_data_.charging = is_charging_;
    for (auto& pair : subscriptions_chg_) {
        if (pair.second.cb != NULL && IsOverThreshold(pair.second, last_chg_data_)) {
            pair.second.cb(last_chg_data_);
            pair.second.last_notification_ = last_chg_data_;
        }
    }
}

void BatteryManager::TimerQueueWork(struct k_timer* timer)
{
    k_work* pending_work = reinterpret_cast<k_work*>(k_timer_user_data_get(timer));
    k_work_submit(pending_work);
}

void BatteryManager::HandleBatteryGeneralDataCallback(struct k_work* work)
{
    k_work_wrapper<BatteryManager>* wrapper =
        CONTAINER_OF(work, k_work_wrapper<BatteryManager>, work);
    BatteryManager* self = wrapper->self;
    self->HandleBatteryGeneralData();
}

void BatteryManager::HandleBatteryChargingDataCallback(struct k_work* work)
{
    k_work_wrapper<BatteryManager>* wrapper =
        CONTAINER_OF(work, k_work_wrapper<BatteryManager>, work);
    BatteryManager* self = wrapper->self;
    self->HandleBatteryChargingData();
}

bool BatteryManager::IsOverThreshold(const GeneralSubscription& subscription,
                                     const BatteryGeneralData& last_sample) const
{
    if ((fabs(subscription.last_notificaton_.temp - last_sample.temp) >=
         subscription.thresholds.temp) ||
        (fabs(subscription.last_notificaton_.current - last_sample.current) >=
         subscription.thresholds.current) ||
        (fabs(subscription.last_notificaton_.volt - last_sample.volt) >=
         subscription.thresholds.volt) ||
        (fabs(subscription.last_notificaton_.remaining_capacity - last_sample.remaining_capacity) >=
         subscription.thresholds.remaining_capacity) ||
        (abs(subscription.last_notificaton_.cycle_count - last_sample.cycle_count) >=
         subscription.thresholds.cycle_count)) {
        return true;
    }

    return false;
}

bool BatteryManager::IsOverThreshold(const ChargingSubscription& subscription,
                                     const BatteryChargingData& last_sample) const
{
    if ((subscription.last_notification_.status != last_sample.status) ||
        (subscription.last_notification_.charging != last_sample.charging) ||
        (abs(subscription.last_notification_.relative_charge_state -
             last_sample.relative_charge_state) >= subscription.thresholds.relative_charge_state) ||
        (abs(subscription.last_notification_.des_chg_current - last_sample.des_chg_current) >=
         subscription.thresholds.des_chg_current) ||
        (abs(subscription.last_notification_.des_chg_volt - last_sample.des_chg_volt) >=
         subscription.thresholds.des_chg_volt)) {
        return true;
    }

    return false;
}

bool BatteryManager::SubscriptionTypeIsKnown(std::string subscriber, std::string data_type) const
{
    if (data_type == "GENERAL" && subscriptions_gen_.count(subscriber) >= 1) {
        return true;
    } else if (data_type == "CHARGING" && subscriptions_chg_.count(subscriber) >= 1) {
        return true;
    }

    return false;
}

bool BatteryManager::ModeIsRegistered(const std::string& mode) const
{
    if (chg_limits_.count(mode) < 1) {
        return false;
    }
    return true;
}

bool BatteryManager::ChargingAllowed()
{
    bool chg_rules_passed = last_chg_data_.relative_charge_state <
                            chg_limits_[installation_mode_];  // Manager specific rule
    bool bat_can_be_charged =
        battery_.CanBeCharged(last_chg_data_.status);  // Battery specific requirements

    return bat_can_be_charged && chg_rules_passed;
}