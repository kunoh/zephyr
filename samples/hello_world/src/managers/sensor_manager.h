#pragma once

#include <zephyr/kernel.h>

#include <functional>
#include <vector>

#include "sensor.h"
#include "manager.h"
#include "wrappers_zephyr.h"

class SensorManager : public Manager {
public:
    SensorManager(Sensor& sensor);
    ~SensorManager() = default;
    int Init() override;
    int Selftest() override;
    void AddSubscriber(std::function<void()> cb);
    void StartSampling();
    void StopSampling();

private:
    static void SamplingTimerHandler(struct k_timer* timer);
    static void GetSample(struct k_work* work);

private:
    Sensor& sensor_;
    k_timer timer_;
    k_work_wrapper<SensorManager> work_wrapper_;
    std::vector<std::function<void()>> subscribers_;
};