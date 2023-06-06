#include "inclinometer_manager.h"

#include <inclinometer.h>
#include <logger.h>
#include <zephyr/kernel.h>

#include <functional>
#include <memory>
#include <vector>

#include "util.h"

InclinometerManager::InclinometerManager(std::shared_ptr<Logger> logger,
                                         std::unique_ptr<Inclinometer> inclino)
    : logger_{logger}, inclino_{std::move(inclino)}
{
    last_known_x_angle_ = 0;
    last_known_y_angle_ = 0;
    last_known_z_angle_ = 0;

    // inclino->Init();

    k_timer_init(&timer_, InclinoTimerHandler, NULL);
    k_timer_user_data_set(&timer_, this);

    k_work_init(&work_, ReadInclinoData);
}

void InclinometerManager::InclinoTimerHandler(struct k_timer *timer)
{
    InclinometerManager *self =
        reinterpret_cast<InclinometerManager *>(k_timer_user_data_get(timer));
    k_work_submit(&self->work_);
}

void InclinometerManager::ReadInclinoData(struct k_work *work)
{
    InclinometerManager *self = CONTAINER_OF(work, InclinometerManager, work_);

    double rx_buffer[3];

    // Do inclinometer read here:
    self->inclino_->Read();
    self->inclino_->GetAngle(rx_buffer);
    self->last_known_x_angle_ = rx_buffer[0];
    self->last_known_y_angle_ = rx_buffer[1];
    self->last_known_z_angle_ = rx_buffer[2];
    // printk("InclinometerManager::last_known_x_value_: %u \r\n",self->last_known_x_angle_);

    // Hand latest data to subscribers:
    for (uint16_t i = 0; i < self->subscribers_.size(); i++) {
        (self->subscribers_)[i](self->last_known_x_angle_);
    }
}

void InclinometerManager::StartInclinoTimer()
{
    k_timer_start(&timer_, K_MSEC(2000), K_MSEC(2000));
}

void InclinometerManager::StopInclinoTimer()
{
    k_timer_stop(&timer_);
}

uint32_t InclinometerManager::GetLastXAngle()
{
    return last_known_x_angle_;
}
uint32_t InclinometerManager::GetLastYAngle()
{
    return last_known_y_angle_;
}
uint32_t InclinometerManager::GetLastZAngle()
{
    return last_known_z_angle_;
}

bool InclinometerManager::ChangeTimer(uint32_t new_time_ms)
{
    // Set timer to new time
    k_timer_start(&timer_, K_MSEC(500), K_MSEC(new_time_ms));
    return 0;
}

bool InclinometerManager::Subscribe(std::function<int(uint32_t)> new_sub)
{
    subscribers_.push_back(new_sub);

    return true;
}

uint32_t InclinometerManager::GetSubscribeCount(void)
{
    return (uint32_t)subscribers_.size();
}