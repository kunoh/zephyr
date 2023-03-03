#include "imu_manager.h"

ImuManager::ImuManager(std::shared_ptr<Logger> logger, std::unique_ptr<Imu> imu)
    : logger_{logger}, imu_{std::move(imu)}
{
    k_timer_init(&timer_, SamplingTimerHandler, NULL);
    k_timer_user_data_set(&timer_, this);

    k_work_init(&work_, GetSample);
}

void ImuManager::AddSubscriber(std::function<void(ImuSampleData)> cb)
{
    callbacks_.push_back(cb);
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
    k_work_submit(&self->work_);
}

void ImuManager::GetSample(struct k_work *work)
{
    ImuManager *self = CONTAINER_OF(work, ImuManager, work_);
    ImuSampleData sample_data;

    self->imu_->FetchSampleData();
    self->imu_->GetAccData(sample_data.acc.x, sample_data.acc.y, sample_data.acc.z);
    self->imu_->GetMagData(sample_data.mag.x, sample_data.mag.y, sample_data.mag.z);

    for (auto &cb : self->callbacks_) {
        cb(sample_data);
    }
}