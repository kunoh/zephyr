#include "state_manager_sml.h"

StateManager::StateManager(Logger* logger)
: logger_{logger}
{
    k_timer_init(&timer_, RunnerTimerHandler, NULL);
    k_timer_user_data_set(&timer_, this);

    k_work_init(&work_, Running);

    sm_.process_event(Start{});
}

void StateManager::Initialize()
{
    logger_->inf("Finished Initializing");
    logger_->inf("Go to Running");
    sm_.process_event(Success{});
}

void StateManager::StartRunningTimer()
{
    k_timer_start(&timer_, K_MSEC(50), K_MSEC(1000));
}
void StateManager::StopRunningTimer()
{
    k_timer_stop(&timer_);
}

void StateManager::RunnerTimerHandler(k_timer *timer)
{
    StateManager *self = reinterpret_cast<StateManager *>(k_timer_user_data_get(timer));
    k_work_submit(&self->work_);
}

void StateManager::Running(k_work *work)
{
    static int counter = 0;
    counter++;
    StateManager *self = CONTAINER_OF(work, StateManager, work_);
    self->logger_->inf("Do Run");
    if (counter % 3 == 0) {
        self->logger_->inf("Go to Standby");
        self->sm_.process_event(Sleep{});
    }
}

void StateManager::Standby()
{
    logger_->inf("Sleeping");
    sm_.process_event(Init{})
}