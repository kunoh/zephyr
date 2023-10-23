#pragma once

#include <zephyr/kernel.h>

#include <functional>
#include <memory>
#include <vector>

#include "imu.h"
#include "manager.h"
#include "wrappers_zephyr.h"

class ImuManager : public Manager {
public:
    ImuManager(Imu& imu);
    ~ImuManager() = default;
    int Init() override;
    int Selftest() override;
    void AddSubscriber(std::function<void()> cb);
    void StartSampling();
    void StopSampling();

private:
    static void SamplingTimerHandler(struct k_timer* timer);
    static void GetSample(struct k_work* work);

private:
    Imu& imu_;
    k_timer timer_;
    k_work_wrapper<ImuManager> work_wrapper_;
    std::vector<std::function<void()>> subscribers_;
};