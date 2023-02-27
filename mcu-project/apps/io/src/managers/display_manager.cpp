#include "display_manager.h"

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

DisplayManager::DisplayManager(Logger *logger, Display *disp) : logger_{logger}, disp_{disp}
{
    logo_ = logo;
    spinner_ = spinner;

    k_timer_init(&timer_, SpinnerTimerHandler, NULL);
    k_timer_user_data_set(&timer_, this);

    k_work_init(&work_, DoSpin);
}

void DisplayManager::SetBootLogo()
{
    disp_->DisplayWrite((320 - 198) / 2, 10, 198, 16, 198, logo_);
}

void DisplayManager::NextFrame()
{
    disp_->NextFrame();
}

void DisplayManager::StartSpinner()
{
    k_timer_start(&timer_, K_MSEC(50), K_MSEC(200));
}

void DisplayManager::StopSpinner()
{
    k_timer_stop(&timer_);
}

void DisplayManager::SpinnerTimerHandler(struct k_timer *timer)
{
    DisplayManager *self = reinterpret_cast<DisplayManager *>(k_timer_user_data_get(timer));
    k_work_submit(&self->work_);
}

void DisplayManager::DoSpin(struct k_work *work)
{
    static uint32_t spinner_idx = 0;
    DisplayManager *self = CONTAINER_OF(work, DisplayManager, work_);

    uint8_t *spin = (uint8_t *)self->spinner_ + spinner_idx % 8 * (64 * 64 * 3);
    self->disp_->DisplayWrite((320 - 64) / 2, 90, 64, 64, 64, spin);
    spinner_idx++;
}