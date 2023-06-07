#include "imu_manager.h"

ImuManager::ImuManager(Logger &logger, Imu &imu) : logger_{logger}, imu_{imu}
{
    k_timer_init(&timer_, &ImuManager::SamplingTimerHandler, NULL);
    k_timer_user_data_set(&timer_, this);

    k_work_init(&work_wrapper_.work, &ImuManager::GetSampleCallback);
    work_wrapper_.self = this;
}

int ImuManager::Init()
{
    int ret = 0;
    ret = imu_.Init();
    if (ret != 0) {
        return ret;
    }
    StartSampling();
    return ret;
}

void ImuManager::AddErrorCb(void (*cb)(void *), void *user_data)
{
    on_error_.cb = cb;
    on_error_.user_data = user_data;
}

void ImuManager::AddSubscriber(std::function<void(ImuSampleData)> cb)
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

void ImuManager::GetSample()
{
    ImuSampleData sample_data;

    int ret = 0;
    ret = imu_.FetchSampleData();
    if (ret != 0) {
        on_error_.cb(on_error_.user_data);
    }

    imu_.GetAccData(sample_data.acc.x, sample_data.acc.y, sample_data.acc.z);
    imu_.GetMagData(sample_data.mag.x, sample_data.mag.y, sample_data.mag.z);

    for (auto &cb : subscribers_) {
        cb(sample_data);
    }
}

void ImuManager::SamplingTimerHandler(struct k_timer *timer)
{
    ImuManager *self = reinterpret_cast<ImuManager *>(k_timer_user_data_get(timer));
    k_work_submit(&self->work_wrapper_.work);
}

void ImuManager::GetSampleCallback(struct k_work *work)
{
    k_work_wrapper<ImuManager> *wrapper = CONTAINER_OF(work, k_work_wrapper<ImuManager>, work);
    ImuManager *self = wrapper->self;
    self->GetSample();
}