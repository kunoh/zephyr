#include "battery_manager.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(bat_mgr, CONFIG_BATTERY_MANAGER_LOG_LEVEL);

BatteryManager::BatteryManager(Battery &battery, BatteryCharger &charger)
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

    for (int32_t sub_num = 0; sub_num < NUM_SUBSCRIBER_TYPES; sub_num++) {
        for (int32_t data = GENERAL; data != BAT_SENTINEL; data++) {
            subscriptions_.emplace(
                std::make_pair(subscriber_types[sub_num], static_cast<bat_data_t>(data)), false);
        }
    }

    ret = battery_.Init();
    if (ret != 0) {
        return ret;
    }
    StartSampling(GENERAL, GENERAL_INIT_DELAY_MSEC, GENERAL_PERIOD_MSEC);

    ret = charger_.Init();
    if (ret != 0) {
        return ret;
    }
    StartSampling(CHARGING, CHARGING_INIT_DELAY_MSEC, CHARGING_PERIOD_MSEC);

    return 0;
}

void BatteryManager::AddErrorCb(void (*cb)(void *), void *user_data)
{
    on_error_.cb = cb;
    on_error_.user_data = user_data;
}

std::string BatteryManager::GetInstallationMode()
{
    return installation_mode_;
}

int BatteryManager::SetInstallationMode(std::string mode)
{
    if (!ModeIsRegistered(mode)) {
        return EINVAL;
    }

    installation_mode_ = mode;
    return 0;
}

int BatteryManager::GetModeChargingLimit(std::string mode, int32_t &limit)
{
    if (!ModeIsRegistered(mode)) {
        return EINVAL;
    }

    limit = chg_limits_[mode];
    return 0;
}

int BatteryManager::SetModeChargingLimit(std::string mode, int32_t limit)
{
    if (!ModeIsRegistered(mode)) {
        return EINVAL;
    } else if (limit < CHARGING_REL_CHG_STATE_MIN || limit > CHARGING_REL_CHG_STATE_MAX) {
        return ERANGE;
    }

    chg_limits_[mode] = limit;
    return 0;
}

int BatteryManager::AddSubscriberGeneral(std::function<int(BatteryGeneralData)> cb,
                                         std::string subscriber)
{
    if (!SubscriptionTypeIsKnown(subscriber, GENERAL)) {
        return EINVAL;
    } else if (IsSubscribed(subscriber, GENERAL)) {
        return EEXIST;
    }

    subscriber_cbs_gen_.push_back(cb);
    subscriptions_[{subscriber, GENERAL}] = true;
    return 0;
}

int BatteryManager::AddSubscriberCharging(std::function<int(BatteryChargingData)> cb,
                                          std::string subscriber)
{
    if (!SubscriptionTypeIsKnown(subscriber, CHARGING)) {
        return EINVAL;
    } else if (IsSubscribed(subscriber, CHARGING)) {
        return EEXIST;
    }

    subscriber_cbs_chg_.push_back(cb);
    subscriptions_[{subscriber, CHARGING}] = true;
    return 0;
}

size_t BatteryManager::GetSubscriberCbCount(bat_data_t type)
{
    switch (type) {
        case GENERAL:
            return subscriber_cbs_gen_.size();
        case CHARGING:
            return subscriber_cbs_chg_.size();
        default:
            LOG_ERR("Trying to get non-existing type of subscribers.");
            return 0;
    }
}

void BatteryManager::ClearSubscribers(bat_data_t type)
{
    switch (type) {
        case GENERAL:
            subscriber_cbs_gen_.clear();
            for (int i = 0; i < NUM_SUBSCRIBER_TYPES; i++) {
                subscriptions_[{subscriber_types[i], GENERAL}] = false;
            }
            break;

        case CHARGING:
            subscriber_cbs_chg_.clear();
            for (int i = 0; i < NUM_SUBSCRIBER_TYPES; i++) {
                subscriptions_[{subscriber_types[i], CHARGING}] = false;
            }
            break;

        default:
            LOG_ERR("Trying to clear non-existing type of subscribers.");
            break;
    }
}

bool BatteryManager::IsSubscribed(std::string subscriber, bat_data_t data_type)
{
    if (SubscriptionTypeIsKnown(subscriber, data_type)) {
        return subscriptions_[std::make_pair(subscriber, data_type)];
    }
    return false;
}

int BatteryManager::StartSampling(bat_data_t type, uint32_t init_delay_msec, uint32_t period_msec)
{
    switch (type) {
        case GENERAL:
            k_timer_start(&timer_work_bat_gen_data_.first, K_MSEC(init_delay_msec),
                          K_MSEC(period_msec));
            break;

        case CHARGING:
            if (period_msec > CHARGING_PERIOD_UPPER_LIMIT_MSEC) {
                LOG_ERR("Tried to set too slow battery data charging sample rate.");
                return ERANGE;
            }

            k_timer_start(&timer_work_bat_chg_data_.first, K_MSEC(init_delay_msec),
                          K_MSEC(period_msec));
            break;

        default:
            LOG_ERR("Trying to start non-existing bat sampling timer.");
            return EINVAL;
            break;
    }

    return 0;
}

void BatteryManager::StopSampling(bat_data_t type)
{
    switch (type) {
        case GENERAL:
            k_timer_stop(&timer_work_bat_gen_data_.first);
            break;

        case CHARGING:
            k_timer_stop(&timer_work_bat_chg_data_.first);
            break;

        default:
            LOG_ERR("Trying to stop non-existing bat sampling timer.");
            break;
    }
}

int BatteryManager::GetLastGeneralData(BatteryGeneralData &bat_gen_data)
{
    // If sampling timer is stopped, data might be old.
    if (k_timer_remaining_ticks(&timer_work_bat_gen_data_.first) != 0) {
        bat_gen_data.temp = last_bat_gen_data_.temp;
        bat_gen_data.current = last_bat_gen_data_.current;
        bat_gen_data.volt = last_bat_gen_data_.volt;
        bat_gen_data.remaining_capacity = last_bat_gen_data_.remaining_capacity;
        bat_gen_data.cycle_count = last_bat_gen_data_.cycle_count;
    } else {
        bat_gen_data.temp = DEFAULT_INVALID_BAT_FLOAT;
        bat_gen_data.current = DEFAULT_INVALID_BAT_FLOAT;
        bat_gen_data.volt = DEFAULT_INVALID_BAT_FLOAT;
        bat_gen_data.remaining_capacity = DEFAULT_INVALID_BAT_INT;
        bat_gen_data.cycle_count = DEFAULT_INVALID_BAT_INT;
        return EIO;
    }

    return 0;
}

int BatteryManager::GetLastChargingData(BatteryChargingData &bat_chg_data)
{
    // If sampling timer is stopped, data might be old.
    if (k_timer_remaining_ticks(&timer_work_bat_chg_data_.first) != 0) {
        bat_chg_data.des_chg_current = last_bat_chg_data_.des_chg_current;
        bat_chg_data.des_chg_volt = last_bat_chg_data_.des_chg_volt;
        bat_chg_data.status = last_bat_chg_data_.status;
        bat_chg_data.relative_charge_state = last_bat_chg_data_.relative_charge_state;
        bat_chg_data.charging = is_charging_;
    } else {
        bat_chg_data.des_chg_current = DEFAULT_INVALID_BAT_INT;
        bat_chg_data.des_chg_volt = DEFAULT_INVALID_BAT_INT;
        bat_chg_data.status = DEFAULT_INVALID_BAT_INT;
        bat_chg_data.relative_charge_state = DEFAULT_INVALID_BAT_INT;
        bat_chg_data.charging = false;
        return EIO;
    }

    return 0;
}

bool BatteryManager::IsCharging()
{
    return is_charging_;
}

//--- Private member functions ---//

void BatteryManager::HandleBatteryGeneralData()
{
    if (battery_.TriggerGeneralSampling() != 0) {
        LOG_ERR("Failed sampling one or more general battery properties.");
    }
    if (battery_.GetGeneralData(last_bat_gen_data_) != 0) {
        LOG_ERR("Failed fetching one or more general battery properties.");
    }

    // Send to subscribers
    for (auto &cb : subscriber_cbs_gen_) {
        cb(last_bat_gen_data_);
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
        (battery_.GetChargingData(last_bat_chg_data_) != 0)) {
        LOG_WRN("Failed sampling one or more battery charging properties.");
        inhibit_charging = true;
    } else if (!ChargingAllowed()) {
        if (is_charging_) {
            LOG_INF("Battery cannot be charged. Battery status: 0x%X", last_bat_chg_data_.status);
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
        ret = charger_.SetChargingConfig(last_bat_chg_data_.des_chg_current,
                                         last_bat_chg_data_.des_chg_volt);

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
    last_bat_chg_data_.charging = is_charging_;
    for (auto &cb : subscriber_cbs_chg_) {
        cb(last_bat_chg_data_);
    }
}

void BatteryManager::TimerQueueWork(struct k_timer *timer)
{
    k_work *pending_work = reinterpret_cast<k_work *>(k_timer_user_data_get(timer));
    k_work_submit(pending_work);
}

void BatteryManager::HandleBatteryGeneralDataCallback(struct k_work *work)
{
    k_work_wrapper<BatteryManager> *wrapper =
        CONTAINER_OF(work, k_work_wrapper<BatteryManager>, work);
    BatteryManager *self = wrapper->self;
    self->HandleBatteryGeneralData();
}

void BatteryManager::HandleBatteryChargingDataCallback(struct k_work *work)
{
    k_work_wrapper<BatteryManager> *wrapper =
        CONTAINER_OF(work, k_work_wrapper<BatteryManager>, work);
    BatteryManager *self = wrapper->self;
    self->HandleBatteryChargingData();
}

bool BatteryManager::SubscriptionTypeIsKnown(std::string sub, bat_data_t data_type)
{
    if (subscriptions_.count(std::make_pair(sub, data_type)) < 1) {
        return false;
    }
    return true;
}

bool BatteryManager::ModeIsRegistered(std::string mode)
{
    if (chg_limits_.count(mode) < 1) {
        return false;
    }
    return true;
}

bool BatteryManager::ChargingAllowed()
{
    bool chg_rules_passed = last_bat_chg_data_.relative_charge_state <
                            chg_limits_[installation_mode_];  // Manager specific rule
    bool bat_can_be_charged =
        battery_.CanBeCharged(last_bat_chg_data_.status);  // Battery specific requirements

    return bat_can_be_charged && chg_rules_passed;
}