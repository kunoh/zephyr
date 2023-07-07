#include "inclinometer_manager.h"

#include <zephyr/logging/log.h>

#include "util.h"

LOG_MODULE_REGISTER(incl_mgr, CONFIG_INCLINOMETER_MANAGER_LOG_LEVEL);

InclinometerManager::InclinometerManager(Inclinometer &inclino) : inclino_{inclino}
{
    last_known_x_angle_ = 0;
    last_known_y_angle_ = 0;
    last_known_z_angle_ = 0;

    k_timer_init(&timer_, &InclinometerManager::InclinoTimerHandler, NULL);
    k_timer_user_data_set(&timer_, this);

    k_work_init(&work_wrapper_.work, &InclinometerManager::ReadInclinoDataCallback);
    work_wrapper_.self = this;
}

int InclinometerManager::Init()
{
    int ret = 0;
    ret = inclino_.Init();
    if (ret != 0) {
        return ret;
    }
    sample_period_ = 0;
    StartInclinoTimer();
    return 0;
}

int InclinometerManager::Selftest()
{
    // Do selftest
    LOG_INF("Inclinometer manager OK");
    return 0;
}

void InclinometerManager::AddErrorCb(void (*cb)(void *), void *user_data)
{
    on_error_.cb = cb;
    on_error_.user_data = user_data;
}

float InclinometerManager::GetLastXAngle()
{
    return last_known_x_angle_;
}
float InclinometerManager::GetLastYAngle()
{
    return last_known_y_angle_;
}
float InclinometerManager::GetLastZAngle()
{
    return last_known_z_angle_;
}

/// @brief Set sample timer timout interval in ms.
/// @param new_time_ms New sample timer timeout in ms.
/// @return True
bool InclinometerManager::ChangeTimer(uint32_t new_time_ms)
{
    // Set timer to new time
    k_timer_start(&timer_, K_MSEC(500), K_MSEC(new_time_ms));
    sample_period_ = new_time_ms;
    return true;
}

/// @brief Start the sample timer.
///
void InclinometerManager::StartInclinoTimer()
{
    sample_period_ = 2000;
    k_timer_start(&timer_, K_MSEC(2000), K_MSEC(2000));
}

/// @brief Stop the sample timer.
///
void InclinometerManager::StopInclinoTimer()
{
    k_timer_stop(&timer_);
}

/// @brief  Returns the period of the sample timer in ms.
/// @return Sample timer timeout in ms.
uint32_t InclinometerManager::GetSamplePeriod()
{
    return sample_period_;
}

bool InclinometerManager::Subscribe(std::function<int(SensorSampleData)> new_sub)
{
    subscribers_.push_back(new_sub);

    return true;
}

uint32_t InclinometerManager::GetSubscribeCount(void)
{
    return (uint32_t)subscribers_.size();
}

void InclinometerManager::ReadInclinoData()
{
    SensorSampleData rx_buffer;

    // Do inclinometer read here:
    inclino_.Read();
    inclino_.GetAngle(&rx_buffer);
    last_known_x_angle_ = rx_buffer.x;
    last_known_y_angle_ = rx_buffer.y;
    last_known_z_angle_ = rx_buffer.z;

    // Hand latest data to subscribers:
    for (auto &cb : subscribers_) {
        cb(rx_buffer);
    }
}

void InclinometerManager::InclinoTimerHandler(struct k_timer *timer)
{
    InclinometerManager *self =
        reinterpret_cast<InclinometerManager *>(k_timer_user_data_get(timer));
    k_work_submit(&self->work_wrapper_.work);
}

void InclinometerManager::ReadInclinoDataCallback(struct k_work *work)
{
    k_work_wrapper<InclinometerManager> *wrapper =
        CONTAINER_OF(work, k_work_wrapper<InclinometerManager>, work);
    InclinometerManager *self = wrapper->self;
    self->ReadInclinoData();
}