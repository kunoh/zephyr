#include "leg_manager.h"

#include <zephyr/logging/log.h>

#include "util.h"

LOG_MODULE_REGISTER(leg_mgr, CONFIG_LEG_MANAGER_LOG_LEVEL);

LegManager::LegManager(LegControl &leg_control, InclinometerManager &incl_mgr)
    : leg_control_{leg_control}, incl_mgr_{incl_mgr}
{
    last_known_value_ = 0;

    k_timer_init(&timer_, &LegManager::LegTimerHandler, NULL);
    k_timer_user_data_set(&timer_, this);

    k_work_init(&work_wrapper_.work, &LegManager::DoLegWorkCallback);
    work_wrapper_.self = this;
}

int LegManager::Init()
{
    if (!leg_control_.Init()) {
        LOG_ERR("Failed to initialize legs");
        return 1;
    }
    if (!incl_mgr_.Subscribe(&LegManager::SubscribeCallback)) {
        LOG_ERR("Could not subscribe to Inclinometer");
        return 1;
    }
    return 0;
}

int LegManager::Selftest()
{
    // Do selftest
    LOG_INF("Leg manager OK");
    return 0;
}

void LegManager::AddErrorCb(void (*cb)(void *), void *user_data)
{
    on_error_.cb = cb;
    on_error_.user_data = user_data;
}

void LegManager::StartLegTimer()
{
    k_timer_start(&timer_, K_MSEC(550), K_MSEC(1000));
}

void LegManager::StopLegTimer()
{
    k_timer_stop(&timer_);
}

uint32_t LegManager::GetLastValue()
{
    return last_known_value_;
}

bool LegManager::ChangeTimer(uint32_t new_time_ms)
{
    // Set timer to new time
    k_timer_start(&timer_, K_MSEC(500), K_MSEC(new_time_ms));
    return 0;
}

int LegManager::SubscribeCallback(SensorSampleData data)
{
    // printk("SUB FROM LEG! data: %f\r\n", data.x);
    return 0;
}

void LegManager::DoLegWork()
{
    return;
}

void LegManager::LegTimerHandler(struct k_timer *timer)
{
    LegManager *self = reinterpret_cast<LegManager *>(k_timer_user_data_get(timer));
    k_work_submit(&self->work_wrapper_.work);
}

void LegManager::DoLegWorkCallback(struct k_work *work)
{
    k_work_wrapper<LegManager> *wrapper = CONTAINER_OF(work, k_work_wrapper<LegManager>, work);
    LegManager *self = wrapper->self;
    self->DoLegWork();
}