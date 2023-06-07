#include "state_manager_sml.h"

StateManagerSml::StateManagerSml(Logger &logger)
    : logger_{logger}, sm_{static_cast<FsmOps &>(*this)}
{
    k_work_init(&work_wrapper_.work, &StateManagerSml::ProcessStartEvent);
    work_wrapper_.self = this;
}

void StateManagerSml::AddManager(Manager &m)
{
    managers_.push_back(&m);
}

void StateManagerSml::Run()
{
    k_work_submit(&work_wrapper_.work);
}

// Init state
void StateManagerSml::Initialize()
{
    logger_.inf("Initializing");
    for (auto m : managers_) {
        m->AddErrorCb(&StateManagerSml::OnError, this);

        if (m->Init() != 0) {
            sm_.process_event(Failed{});
        }
    }
    sm_.process_event(Success{});
}

void StateManagerSml::Selftest()
{
    logger_.inf("Selftest");
    // Do something...
    sm_.process_event(Success{});
}

// Ready state
void StateManagerSml::Ready()
{
    logger_.inf("Ready");
    // Do something...
}

// Standby state
void StateManagerSml::Standby()
{
    logger_.inf("Standby");
    // Power down or put devices to sleep
}

void StateManagerSml::Reset()
{
    logger_.inf("Reset");
    // Do something...
}

void StateManagerSml::Error()
{
    logger_.inf("Error");
    // Do something...
}

void StateManagerSml::OnError(void *user_data)
{
    StateManagerSml *self = reinterpret_cast<StateManagerSml *>(user_data);
    self->sm_.process_event(Failed{});
}

void StateManagerSml::ProcessStartEvent(k_work *work)
{
    k_work_wrapper<StateManagerSml> *wrapper =
        CONTAINER_OF(work, k_work_wrapper<StateManagerSml>, work);
    StateManagerSml *self = wrapper->self;
    self->sm_.process_event(Start{});
}