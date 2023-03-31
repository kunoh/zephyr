#include <zephyr.h>

#include "boost/sml.hpp"
#include "logger.h"

// clang-format off

namespace sml = boost::sml;

// States
struct Off {};
struct Init {};
struct Running {};
struct Standby {};
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
    virtual void StartRunningTimer() = 0;
    virtual void StopRunningTimer() = 0;
    virtual void Standby() = 0;
};

struct Fsm {
    auto operator()() const {
        using namespace boost::sml;

        return make_transition_table(
            *state<Off>    + event<Start>   / [](FsmOps& fo){ fo.Initialize(); }         = state<Init>,
            state<Init>    + event<Success>                                              = state<Running>,
            state<Running> + on_entry<_>    / [](FsmOps& fo){ fo.StartRunningTimer(); },
            state<Running> + on_exit<_>     / [](FsmOps& fo){ fo.StopRunningTimer(); },
            state<Running> + event<Sleep>   / [](FsmOps& fo){ fo.Standby(); }            = state<Standby>,
            state<Standby> + event<Start>   / [](FsmOps& fo){ fo.Initialize(); }         = state<Init>

            //state<Init>    + event<Failed> / [](){...} = state<Error>,
            //state<Running> + event<Failed> / [](){...} = state<Error>,
        );
    }
};

// clang-format on

class StateManager : public FsmOps {
public:
    StateManager(Logger* logger);
    void Initialize();
    void StartRunningTimer();
    void StopRunningTimer();
    void Standby();

private:
    void StartRunner();
    void StopRunner();
    static void RunnerTimerHandler(k_timer *timer);
    static void Running(k_work *work);

private:
    boost::sml::sm<Fsm> sm_;
    Logger* logger_;

    k_timer timer_;
    k_work work_;
};