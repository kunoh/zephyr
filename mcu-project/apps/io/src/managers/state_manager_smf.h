#include <zephyr.h>
#include <zephyr/smf.h>

/* User defined object */
struct s_object {
        /* This must be first */
        smf_ctx ctx;

        /* Other state specific data add here */
};

class StateManager {
public:
    StateManager();
    ~StateManager() = default;

private:
    void StartRunner();
    void StopRunner();
    static void RunnerTimerHandler(k_timer *timer);
    static void Runner(k_work *work);
    static void Init(void *o);
    static void RunningEntry(void *o);
    static void Running(void *o);
    static void RunningExit(void *o);
    static void Standby(void *o);

private:
    /* List of demo states */
    enum state_ { INIT, RUNNING, STANDBY };

    /* Populate state table */
    const smf_state states_[3] = {
            [INIT] = SMF_CREATE_STATE(NULL, Init, NULL),
            [RUNNING] = SMF_CREATE_STATE(RunningEntry, Running, RunningExit),
            [STANDBY] = SMF_CREATE_STATE(NULL, Standby, NULL),
    };

    s_object s_obj_;

    k_timer timer_;
    k_work work_;
};