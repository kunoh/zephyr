#pragma once

class DisplayManager {
public:
    DisplayManager(Logger* logger, Display* disp);
    ~DisplayManager() = default;
    void SetBootLogo();
    void NextFrame();
    void StartSpinner();
    void StopSpinner();

private:
    static void SpinnerTimerHandler(struct k_timer *timer);
    static void DoSpin(struct k_work *work);

private:
    Logger* logger_;
    Display* disp_;
    k_timer timer_;
    k_work work_;

    // Graphic Resources
    const uint8_t *logo_;
    const uint8_t *spinner_;
};