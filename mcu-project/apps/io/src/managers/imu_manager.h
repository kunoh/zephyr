#pragma once
#include <imu.h>
#include <logger.h>
#include <util.h>
#include <zephyr.h>

#include <functional>
#include <memory>
#include <vector>

class ImuManager {
public:
    ImuManager(std::shared_ptr<Logger> logger, std::unique_ptr<Imu> imu);
    ~ImuManager() = default;
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
    k_work work_;
    std::vector<std::function<void(ImuSampleData)> > callbacks_;
};