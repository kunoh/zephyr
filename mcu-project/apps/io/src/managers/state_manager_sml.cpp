#include "state_manager_sml.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sml_mgr, CONFIG_SML_MANAGER_LOG_LEVEL);

StateManagerSml::StateManagerSml() : sm_{static_cast<FsmOps &>(*this)}
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
    LOG_INF("Initializing");
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
    LOG_INF("Selftest");
    // Do something...
    sm_.process_event(Success{});
}

// Ready state
void StateManagerSml::Ready()
{
    LOG_INF("Ready");
    // Do something...
}

// Standby state
void StateManagerSml::Standby()
{
    LOG_INF("Standby");
    // Power down or put devices to sleep
}

void StateManagerSml::Reset()
{
    LOG_INF("Reset");
    // Do something...
}

void StateManagerSml::Error()
{
    LOG_INF("Error");
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