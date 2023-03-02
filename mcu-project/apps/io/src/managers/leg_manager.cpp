#include "leg_manager.h"

#include <logger.h>
#include <zephyr/kernel.h>

#include "inclinometer_manager.h"
#include "leg_control.h"

LegManager::LegManager(std::shared_ptr<Logger> logger, LegControl *leg_control)
    : logger_{logger}, leg_control_{leg_control}
{
    last_known_value_ = 0;

    leg_control->Init();

    k_timer_init(&timer_, LegTimerHandler, NULL);
    k_timer_user_data_set(&timer_, this);

    k_work_init(&work_, DoLegWork);
}

void LegManager::LegTimerHandler(struct k_timer *timer)
{
    LegManager *self = reinterpret_cast<LegManager *>(k_timer_user_data_get(timer));
    k_work_submit(&self->work_);
}

void LegManager::DoLegWork(struct k_work *work)
{
    LegManager *self = CONTAINER_OF(work, LegManager, work_);

    // Do leg_object stuff here:
    // printk("LegManager::last_known_value_: %u \r\n", self->last_known_value_);
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

int LegManager::SubscribeCallback(uint32_t data)
{
    // printk("SUB FROM LEG! data: %u\r\n",data);
    return 0;
}

bool LegManager::InitSubscribtions(InclinometerManager *inclino_man)
{
    inclino_man->Subscribe((LegManager::SubscribeCallback));
    return true;
}