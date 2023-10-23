#include "sensor_manager.h"
#include "zephyr/sys/printk.h"

SensorManager::SensorManager(Sensor& sensor)
    : sensor_{sensor}
{
    k_timer_init(&timer_, SamplingTimerHandler, NULL);
    k_timer_user_data_set(&timer_, this);

    k_work_init(&work_wrapper_.work, GetSample);
    work_wrapper_.self = this;
}

int SensorManager::Init()
{
    int ret = 0;
    ret = sensor_.Init();
    if (ret == 0) {
        StartSampling();
    }
    return ret;
}

int SensorManager::Selftest()
{
    // Do selftest
    printk("Sensor manager OK");
    return 0;
}

void SensorManager::AddSubscriber(std::function<void()> cb)
{
    subscribers_.push_back(cb);
}

void SensorManager::StartSampling()
{
    k_timer_start(&timer_, K_MSEC(50), K_MSEC(500));
}

void SensorManager::StopSampling()
{
    k_timer_stop(&timer_);
}

void SensorManager::SamplingTimerHandler(struct k_timer *timer)
{
    SensorManager *self = reinterpret_cast<SensorManager *>(k_timer_user_data_get(timer));
    k_work_submit(&self->work_wrapper_.work);
}

void SensorManager::GetSample(struct k_work *work)
{
    k_work_wrapper<SensorManager> *wrapper = CONTAINER_OF(work, k_work_wrapper<SensorManager>, work);
    SensorManager *self = wrapper->self;
    float val = 0;

    int ret = 0;
    ret = self->sensor_.FetchSampleData();
    if (ret != 0){
        printk("Fetch sensor sample data failed\n");
    }

    self->sensor_.GetSensorValue(val);

    for (auto &cb : self->subscribers_) {
        cb();
    }
}