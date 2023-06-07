#include "battery_manager.h"

BatteryManager::BatteryManager(Logger &logger, Battery &battery, BatteryCharger &charger)
    : logger_{logger}, battery_{battery}, charger_{charger}
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

    ret = battery_.Init();
    if (ret != 0) {
        return ret;
    }
    StartSampling(GENERAL, 50, 60000);

    ret = charger_.Init();
    if (ret != 0) {
        return ret;
    }
    StartSampling(CHARGING, 50, 2500);

    return 0;
}

void BatteryManager::AddErrorCb(void (*cb)(void *), void *user_data)
{
    on_error_.cb = cb;
    on_error_.user_data = user_data;
}

void BatteryManager::HandleBatteryGeneralData()
{
    if (battery_.TriggerGeneralSampling() != 0) {
        logger_.err("Failed sampling one or more general battery properties.");
    }
    if (battery_.GetGeneralData(last_bat_gen_data_) != 0) {
        logger_.err("Failed fetching one or more general battery properties.");
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
        logger_.wrn("Failed sampling one or more battery charging properties.");
        inhibit_charging = true;
    } else if (!battery_.CanBeCharged(last_bat_chg_data_.status)) {
        logger_.inf("Battery cannot be charged.");
        inhibit_charging = true;
    }

    if (inhibit_charging) {
        ret = charger_.SetChargingCurrent(0);
        if (ret != 0) {
            logger_.err("Couldn't inhibit charging!");
        } else {
            logger_.inf("Inhibited charging.");
            is_charging_ = false;
        }
    } else {
        // Attempt to configure charge controller with charging properties from battery.
        ret = charger_.SetChargingConfig(last_bat_chg_data_.des_chg_current,
                                         last_bat_chg_data_.des_chg_volt);

        if (ret != 0) {
            switch (ret) {
                case ERRNO_CHARGER_VOLTAGE_EIO:
                    logger_.err(
                        "An IO-error occured while attempting to set max charging voltage.");
                    break;
                case ERRNO_CHARGER_VOLTAGE_ERANGE:
                    logger_.err(
                        "Attempting to set max charging voltage outside operable charger range.");
                    break;
                case ERRNO_CHARGER_CURRENT_EIO:
                    logger_.err(
                        "An IO-error occured while attempting to set max charging current.");
                    break;
                case ERRNO_CHARGER_CURRENT_ERANGE:
                    logger_.err(
                        "Attempting to set charging current outside operable charger range.");
                    break;
                default:
                    logger_.err("SetChargingConfig() returned non-zero errno.");
                    break;
            }
        } else {
            is_charging_ = true;
        }
    }

    // Send to subscribers
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

int BatteryManager::StartSampling(bat_data_t type, uint32_t init_delay_msec, uint32_t period_msec)
{
    switch (type) {
        case GENERAL:
            k_timer_start(&timer_work_bat_gen_data_.first, K_MSEC(init_delay_msec),
                          K_MSEC(period_msec));
            break;

        case CHARGING:
            if (period_msec > CHARGING_PERIOD_UPPER_LIMIT_MSEC) {
                logger_.err("Tried to set too slow battery data charging sample rate.");
                return ERANGE;
            }

            k_timer_start(&timer_work_bat_chg_data_.first, K_MSEC(init_delay_msec),
                          K_MSEC(period_msec));
            break;

        default:
            logger_.err("Trying to start non-existing bat sampling timer.");
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
            logger_.err("Trying to stop non-existing bat sampling timer.");
            break;
    }
}

void BatteryManager::AddSubscriberGeneral(std::function<void(BatteryGeneralData)> cb)
{
    subscriber_cbs_gen_.push_back(cb);
}

void BatteryManager::AddSubscriberCharging(std::function<void(BatteryChargingData)> cb)
{
    subscriber_cbs_chg_.push_back(cb);
}

size_t BatteryManager::GetSubscriberCount(bat_data_t type)
{
    switch (type) {
        case GENERAL:
            return subscriber_cbs_gen_.size();
        case CHARGING:
            return subscriber_cbs_chg_.size();
        default:
            logger_.err("Trying to clear non-existing type of subscribers.");
            return 0;
    }
}

void BatteryManager::ClearSubscribers(bat_data_t type)
{
    switch (type) {
        case GENERAL:
            subscriber_cbs_gen_.clear();
            cpu_subscribed_ = false;
            break;
        case CHARGING:
            subscriber_cbs_chg_.clear();
            break;
        default:
            logger_.err("Trying to clear non-existing type of subscribers.");
            break;
    }
}

void BatteryManager::SetCpuSubscribed(bool val)
{
    cpu_subscribed_ = val;
}

bool BatteryManager::CpuIsSubscribed()
{
    return cpu_subscribed_;
}

bool BatteryManager::IsCharging()
{
    return is_charging_;
}

int BatteryManager::GetLastGeneralData(BatteryGeneralData &bat_gen_data)
{
    // If sampling timer is stopped, data might be old.
    if (k_timer_remaining_ticks(&timer_work_bat_gen_data_.first) != 0) {
        bat_gen_data.temp = last_bat_gen_data_.temp;
        bat_gen_data.current = last_bat_gen_data_.current;
        bat_gen_data.volt = last_bat_gen_data_.volt;
        bat_gen_data.remaining_capacity = last_bat_gen_data_.remaining_capacity;
        bat_gen_data.relative_charge_state = last_bat_gen_data_.relative_charge_state;
        bat_gen_data.cycle_count = last_bat_gen_data_.cycle_count;
    } else {
        bat_gen_data.temp = DEFAULT_INVALID_BAT_FLOAT;
        bat_gen_data.current = DEFAULT_INVALID_BAT_FLOAT;
        bat_gen_data.volt = DEFAULT_INVALID_BAT_FLOAT;
        bat_gen_data.remaining_capacity = DEFAULT_INVALID_BAT_INT;
        bat_gen_data.relative_charge_state = DEFAULT_INVALID_BAT_INT;
        bat_gen_data.cycle_count = DEFAULT_INVALID_BAT_INT;
        return -EIO;
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
        bat_chg_data.charging = last_bat_chg_data_.charging;
    } else {
        bat_chg_data.des_chg_current = DEFAULT_INVALID_BAT_INT;
        bat_chg_data.des_chg_volt = DEFAULT_INVALID_BAT_INT;
        bat_chg_data.status = DEFAULT_INVALID_BAT_INT;
        bat_chg_data.charging = false;
    }

    return 0;
}