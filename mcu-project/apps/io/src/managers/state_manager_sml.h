#include <zephyr/kernel.h>

#include <memory>
#include <vector>

#include "boost/sml.hpp"
#include "logger.h"
#include "manager.h"
#include "wrappers_zephyr.h"

// clang-format off

// States
struct Off {};
struct Init {};
struct Selftest{};
struct Ready {};
struct Standby {};
struct Reset {};
struct Error {};

// Events
struct Start {};
struct Success {};
struct Sleep {};
struct Failed {};

// Operations (actions/guards) called from state machine
class FsmOps {
public:
    virtual void Initialize() = 0;
    virtual void Selftest() = 0;
    virtual void Ready() = 0;
    virtual void Standby() = 0;
    virtual void Reset() = 0;
    virtual void Error() = 0;
};

struct Fsm {
    auto operator()() const {
        using namespace boost::sml;

        return make_transition_table(
            *state<Off>      + event<Start>   / [](FsmOps& fo){ fo.Initialize(); }         = state<Init>,
            state<Init>      + event<Success> / [](FsmOps& fo){ fo.Selftest(); }           = state<Selftest>,
            state<Init>      + event<Failed>  / [](FsmOps& fo){ fo.Error(); }              = state<Error>,
            state<Selftest>  + event<Success> / [](FsmOps& fo){ fo.Ready(); }              = state<Ready>,
            state<Selftest>  + event<Failed>  / [](FsmOps& fo){ fo.Error(); }              = state<Error>,
            state<Ready>     + event<Sleep>   / [](FsmOps& fo){ fo.Standby(); }            = state<Standby>,
            state<Ready>     + event<Failed>  / [](FsmOps& fo){ fo.Error(); }              = state<Error>,
            state<Standby>   + event<Start>   / [](FsmOps& fo){ fo.Initialize(); }         = state<Init>,
            state<Standby>   + event<Failed>  / [](FsmOps& fo){ fo.Error(); }              = state<Error>
            //state<Error>     + event<Success> / [](FsmOps& fo){ fo.Ready(); }              = state<Ready>,
            //state<Error>     + event<Failed>  / [](FsmOps& fo){ fo.Error(); }              = state<Reset>
        );
    }
};

// clang-format on

class StateManagerSml : public FsmOps {
public:
    StateManagerSml(Logger& logger);
    ~StateManagerSml() = default;
    void AddManager(Manager& m);
    void Run();
    void Initialize();
    void Selftest();
    void Ready();
    void Standby();
    void Reset();
    void Error();
    static void OnError(void* user_data);

private:
    static void ProcessStartEvent(k_work* work);

private:
    Logger& logger_;
    k_work_wrapper<StateManagerSml> work_wrapper_;
    std::vector<Manager*> managers_;
    boost::sml::sm<Fsm> sm_;
};