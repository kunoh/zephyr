#pragma once

#include <zephyr/kernel.h>

#include <functional>
#include <memory>
#include <vector>

#include "imu.h"
#include "logger.h"
#include "manager.h"
#include "util.h"
#include "wrappers_zephyr.h"

class ImuManager : public Manager {
public:
    ImuManager(std::shared_ptr<Logger> logger, std::unique_ptr<Imu> imu);
    ~ImuManager() = default;
    int Init() override;
    void AddErrorCb(void (*cb)(void*), void* user_data) override;
    void AddSubscriber(std::function<void(ImuSampleData)> cb);
    void StartSampling();
    void StopSampling();

private:
    static void SamplingTimerHandler(struct k_timer* timer);
    static void GetSample(struct k_work* work);

private:
    std::shared_ptr<Logger> logger_;
    std::unique_ptr<Imu> imu_;
    k_timer timer_;
    k_work_wrapper<ImuManager> work_wrapper_;
    CbWrapper on_error_{.user_data = NULL, .cb = NULL};
    std::vector<std::function<void(ImuSampleData)> > subscribers_;
};