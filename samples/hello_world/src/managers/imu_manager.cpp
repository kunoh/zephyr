#include "imu_manager.h"
#include "zephyr/sys/printk.h"

ImuManager::ImuManager(Imu& imu)
    : imu_{imu}
{
    k_timer_init(&timer_, SamplingTimerHandler, NULL);
    k_timer_user_data_set(&timer_, this);

    k_work_init(&work_wrapper_.work, GetSample);
    work_wrapper_.self = this;
}

int ImuManager::Init()
{
    int ret = 0;
    ret = imu_.Init();
    if (ret == 0) {
        StartSampling();
    }
    return ret;
}

int ImuManager::Selftest()
{
    // Do selftest
    printk("IMU manager OK");
    return 0;
}

void ImuManager::AddSubscriber(std::function<void()> cb)
{
    subscribers_.push_back(cb);
}

void ImuManager::StartSampling()
{
    k_timer_start(&timer_, K_MSEC(50), K_MSEC(1000));
}

void ImuManager::StopSampling()
{
    k_timer_stop(&timer_);
}

void ImuManager::SamplingTimerHandler(struct k_timer *timer)
{
    ImuManager *self = reinterpret_cast<ImuManager *>(k_timer_user_data_get(timer));
    k_work_submit(&self->work_wrapper_.work);
}

void ImuManager::GetSample(struct k_work *work)
{
    k_work_wrapper<ImuManager> *wrapper = CONTAINER_OF(work, k_work_wrapper<ImuManager>, work);
    ImuManager *self = wrapper->self;

    int ret = 0;
    ret = self->imu_.FetchSampleData();
    if (ret != 0){
        printk("Fetch imu sample data failed\n");
    }

    double accx, accy, accz;
    double magx, magy, magz;

    self->imu_.GetAccData(accx, accy, accz);
    self->imu_.GetMagData(magx, magy, magz);

    for (auto &cb : self->subscribers_) {
        cb();
    }
}