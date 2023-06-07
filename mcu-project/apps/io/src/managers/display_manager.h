#pragma once

#include <zephyr/kernel.h>

#include "display.h"
#include "logger.h"
#include "manager.h"
#include "util.h"
#include "wrappers_zephyr.h"

class DisplayManager : public Manager {
public:
    DisplayManager(Logger& logger, Display& disp);
    ~DisplayManager() = default;
    int Init() override;
    void AddErrorCb(void (*cb)(void*), void* user_data) override;
    void SetBootLogo();
    void NextFrame();
    void StartSpinner();
    void StopSpinner();

private:
    void Error();
    void DoSpin();
    static void SpinnerTimerHandler(struct k_timer* timer);
    static void DoSpinCallback(struct k_work* work);

private:
    Logger& logger_;
    Display& disp_;
    k_timer timer_;
    k_work_wrapper<DisplayManager> work_wrapper_;
    CallbackWrapper on_error_;
    // Graphic Resources
    const uint8_t* logo_;
    const uint8_t* spinner_;
};