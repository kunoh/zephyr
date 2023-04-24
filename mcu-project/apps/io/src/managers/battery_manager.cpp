#include "battery_manager.h"

BatteryManager::BatteryManager(std::shared_ptr<Logger> logger, std::unique_ptr<Battery> battery)
    : logger_{logger}, battery_{std::move(battery)}
{
    // Periodic information
    k_timer_init(&timer_work_sample_get_data_.first, TimerQueueWork, NULL);
    k_timer_user_data_set(&timer_work_sample_get_data_.first, &timer_work_sample_get_data_.second);
    k_work_init(&timer_work_sample_get_data_.second, NotifySubscribers);
}

void BatteryManager::AddSubscriber(std::function<void(BatteryData)> cb)
{
    subscriber_cbs_.push_back(cb);
}

size_t BatteryManager::GetSubscriberCount()
{
    return subscriber_cbs_.size();
}

void BatteryManager::ClearSubscribers()
{
    subscriber_cbs_.clear();
    cpu_subscribed_ = false;
}

void BatteryManager::StartSampling(uint32_t init_delay_msec, uint32_t period_msec)
{
    if (init_delay_msec < SAMPLE_TRIGGER_INIT_DELAY_LIMIT_MSEC) {
        init_delay_msec = SAMPLE_TRIGGER_INIT_DELAY_LIMIT_MSEC;
    }
    if (period_msec < SAMPLE_TRIGGER_PERIOD_LIMIT_MSEC) {
        period_msec = SAMPLE_TRIGGER_PERIOD_LIMIT_MSEC;
    }
    k_timer_start(&timer_work_sample_get_data_.first, K_MSEC(init_delay_msec), K_MSEC(period_msec));
}

void BatteryManager::StopSampling()
{
    k_timer_stop(&timer_work_sample_get_data_.first);
}

void BatteryManager::SetCpuSubscribed(bool val)
{
    cpu_subscribed_ = val;
}

bool BatteryManager::GetCpuSubscribed()
{
    return cpu_subscribed_;
}

void BatteryManager::TimerQueueWork(struct k_timer *timer)
{
    k_work *pending_work = reinterpret_cast<k_work *>(k_timer_user_data_get(timer));
    k_work_submit(pending_work);
}

int BatteryManager::TriggerAndGetSample(BatteryData *battery_sample_data)
{
    int ret = 0;
    int status = 0;
    ret = ret | (status = battery_->TriggerSampling());
    if (status != 0) {
        logger_->err("Failed to trigger battery sampling.");
    }

    ret = ret | (status = battery_->GetTemperature(&battery_sample_data->temp));
    if (status != 0) {
        logger_->err("Failed to get battery temperature.");
    }

    ret = ret | (status = battery_->GetCurrent(&battery_sample_data->current));
    if (status != 0) {
        logger_->err("Failed to get battery current.");
    }

    ret = ret | (status = battery_->GetVoltage(&battery_sample_data->volt));
    if (status != 0) {
        logger_->err("Failed to get battery voltage.");
    }

    ret = ret | (status = battery_->GetRemCapacity(&battery_sample_data->remaining_capacity));
    if (status != 0) {
        logger_->err("Failed to get remaining battery capacity.");
    }

    ret = ret | (status = battery_->GetStatus(&battery_sample_data->status));
    if (status != 0) {
        logger_->err("Failed to get battery status.");
    }

    ret =
        ret |
        (status = battery_->GetRelativeStateOfCharge(&battery_sample_data->relative_charge_state));
    if (status != 0) {
        logger_->err("Failed to get battery state of charge.");
    }

    ret = ret | (status = battery_->GetCycleCount(&battery_sample_data->cycle_count));
    if (status != 0) {
        logger_->err("Failed to get battery cycle count.");
    }

    return ret;
}

void BatteryManager::NotifySubscribers(struct k_work *work)
{
    BatteryManager *self = CONTAINER_OF(work, BatteryManager, timer_work_sample_get_data_.second);
    BatteryData battery_sample_data;

    self->TriggerAndGetSample(&battery_sample_data);

    for (auto &cb : self->subscriber_cbs_) {
        cb(battery_sample_data);
    }
}