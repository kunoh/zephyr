#include "display_manager.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(disp_mgr, CONFIG_DISPLAY_MANAGER_LOG_LEVEL);

// Graphic Resources
const uint8_t logo[] = {
#include "images/TrackManLogo.h"
};

const uint8_t spinner[] = {
#include "images/Circle1.h"
#include "images/Circle2.h"
#include "images/Circle3.h"
#include "images/Circle4.h"
#include "images/Circle5.h"
#include "images/Circle6.h"
#include "images/Circle7.h"
#include "images/Circle8.h"
};

DisplayManager::DisplayManager(Display &disp) : disp_{disp}
{
    logo_ = logo;
    spinner_ = spinner;

    k_timer_init(&timer_, SpinnerTimerHandler, NULL);
    k_timer_user_data_set(&timer_, this);

    k_work_init(&work_wrapper_.work, &DisplayManager::DoSpinCallback);
    work_wrapper_.self = this;
}

int DisplayManager::Init()
{
    LOG_INF("Display Init");

    SetBootLogo();
    StartSpinner();
    return 0;
}

int DisplayManager::Selftest()
{
    // Do selftest
    LOG_INF("Display manager OK");
    return 0;
}

void DisplayManager::AddErrorCb(void (*cb)(void *), void *user_data)
{
    on_error_.cb = cb;
    on_error_.user_data = user_data;
}

void DisplayManager::SetBootLogo()
{
    disp_.WriteRect((320 - 198) / 2, 10, 198, 16, logo_);
    disp_.DisplayWrite(0, 0, 320, 240, 320);
}

void DisplayManager::NextFrame()
{
    disp_.DisplayWrite(0, 0, 320, 240, 320);
    disp_.NextFrame();
}

void DisplayManager::StartSpinner()
{
    k_timer_start(&timer_, K_MSEC(50), K_MSEC(200));
}

void DisplayManager::StopSpinner()
{
    k_timer_stop(&timer_);
    disp_.NextFrame();
}

void DisplayManager::SpinnerTimerHandler(struct k_timer *timer)
{
    DisplayManager *self = reinterpret_cast<DisplayManager *>(k_timer_user_data_get(timer));
    k_work_submit(&self->work_wrapper_.work);
}

void DisplayManager::DoSpin()
{
    static uint32_t spinner_idx = 0;
    uint8_t *spin = (uint8_t *)spinner_ + spinner_idx % 8 * (64 * 64 * 3);
    int ret = 0;

    disp_.WriteRect((320 - 64) / 2, 90, 64, 64, spin);
    ret = disp_.DisplayWrite(0, 0, 320, 240, 320);
    if (ret != 0) {
        Error();
    }
    spinner_idx++;
}

void DisplayManager::DoSpinCallback(struct k_work *work)
{
    k_work_wrapper<DisplayManager> *wrapper =
        CONTAINER_OF(work, k_work_wrapper<DisplayManager>, work);
    DisplayManager *self = wrapper->self;
    self->DoSpin();
}

void DisplayManager::Error()
{
    if (on_error_.cb != NULL && on_error_.user_data != NULL) {
        on_error_.cb(on_error_.user_data);
    }
}