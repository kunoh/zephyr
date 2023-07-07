#pragma once

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <functional>
#include <vector>

#include "imu.h"
#include "manager.h"
#include "util.h"
#include "wrappers_zephyr.h"

class ImuManager : public Manager {
public:
    ImuManager(Imu& imu);
    ~ImuManager() = default;
    int Init() override;
    int Selftest() override;
    void AddErrorCb(void (*cb)(void*), void* user_data) override;
    void AddSubscriber(std::function<void(ImuSampleData)> cb);
    void StartSampling();
    void StopSampling();

private:
    void GetSample();
    static void SamplingTimerHandler(struct k_timer* timer);
    static void GetSampleCallback(struct k_work* work);

private:
    Imu& imu_;
    k_timer timer_;
    k_work_wrapper<ImuManager> work_wrapper_;
    CallbackWrapper on_error_;
    std::vector<std::function<void(ImuSampleData)> > subscribers_;
};