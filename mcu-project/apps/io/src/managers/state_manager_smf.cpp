#include "state_manager_smf.h"


StateManager::StateManager() {
    k_timer_init(&timer_, RunnerTimerHandler, NULL);
    k_timer_user_data_set(&timer_, this);

    k_work_init(&work_, Runner);

    /* Set initial state */
    smf_set_initial(SMF_CTX(&s_obj_), &states_[INIT]);
    int32_t ret;
    /* State machine terminates if a non-zero value is returned */
    ret = smf_run_state(SMF_CTX(&s_obj_));
    if (ret) {
        /* handle return code and terminate state machine */
    }
}

void StateManager::StartRunner()
{
    k_timer_start(&timer_, K_MSEC(50), K_MSEC(1000));
}

void StateManager::StopRunner()
{
    k_timer_stop(&timer_);
}

void StateManager::RunnerTimerHandler(k_timer *timer)
{
    StateManager *self = reinterpret_cast<StateManager *>(k_timer_user_data_get(timer));
    k_work_submit(&self->work_);
}

/* Run the state machine */
void StateManager::Runner(k_work *work)
{
    StateManager *self = CONTAINER_OF(work, StateManager, work_);
    int32_t ret;
    /* State machine terminates if a non-zero value is returned */
    ret = smf_run_state(SMF_CTX(&self->s_obj_));
    if (ret) {
        /* handle return code and terminate state machine */
    }
}

/* State Init */
void StateManager::Init(void *o)
{
    StateManager *self = CONTAINER_OF(o, StateManager, s_obj_);
    printk("Changing State to: RUNNING\n");
    smf_set_state(SMF_CTX(&self->s_obj_), &self->states_[self->RUNNING]);
}

/* State Running */
void StateManager::RunningEntry(void *o)
{
    printk("State: Standby entry\n");
    StateManager *self = CONTAINER_OF(o, StateManager, s_obj_);
    self->StartRunner();
}

void StateManager::Running(void *o)
{
    static int counter = 0;
    counter++;
    StateManager *self = CONTAINER_OF(o, StateManager, work_);
    printk("Do Run");
    if (counter % 3 == 0) {
        printk("Changing State to: STANDBY\n");
        smf_set_state(SMF_CTX(&self->s_obj_), &self->states_[self->STANDBY]);
    }
}

void StateManager::RunningExit(void *o)
{
    printk("State: Standby exit\n");
    printk("\n\n");
    StateManager *self = CONTAINER_OF(o, StateManager, s_obj_);
    self->StopRunner();
}

/* State Standby */
void StateManager::Standby(void *o)
{
    StateManager *self = CONTAINER_OF(o, StateManager, s_obj_);
    printk("Changing State to: INIT\n");
    smf_set_state(SMF_CTX(&self->s_obj_), &self->states_[self->INIT]);
}
