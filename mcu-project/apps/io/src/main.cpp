#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_device.h>

#include <bitset>

#include "battery_charger_bq25713.h"
#include "battery_charger_mock.h"
#include "battery_manager.h"
#include "battery_message_handler_impl.h"
#include "battery_mock.h"
#include "battery_nh2054qe34.h"
#include "display_com35.h"
#include "display_manager.h"
#include "display_message_handler_impl.h"
#include "display_mock.h"
#include "imu_fxos8700.h"
#include "imu_manager.h"
#include "imu_mock.h"
#include "inclinometer_manager.h"
#include "inclinometer_message_handler_impl.h"
#include "inclinometer_mock.h"
#include "inclinometer_scl3300.h"
#include "leg_control_impl.h"
#include "leg_manager.h"
#include "logger_zephyr.h"
#include "message_dispatcher.h"
#include "message_manager.h"
#include "state_manager_sml.h"
#include "system_message_handler_impl.h"
#include "usb_hid_mock.h"
#include "usb_hid_zephyr.h"
#include "util.h"

LOG_MODULE_REGISTER(main);

int main(void)
{
    LOG_INF("********************************");
    LOG_INF("**  TM5 IO Controller v.%s", "0.13  **");
    LOG_INF("********************************");

    // Initialize Logger
    LoggerZephyr logger("");

    // Initialize External Devices
#if defined(CONFIG_BATTERY_NH2054QE34)
    BatteryNh2054qe34 battery(logger);
#else
    BatteryMock battery;
#endif  // _CONFIG_BATTERY_NH2054QE34_

#if defined(CONFIG_BATTERY_CHARGER_BQ25713)
    BatteryChargerBq25713 charger(logger);
#else
    BatteryChargerMock charger;
#endif

#if defined(CONFIG_DISPLAY)
    DisplayCom35h3 disp(logger);
#else
    DisplayMock disp(logger);
#endif  // !CONFIG_DISPLAY

#if defined(CONFIG_FXOS8700)
    ImuFxos8700 imu(logger);
#else
    ImuMock imu(logger);
#endif  // !CONFIG_FXOS8700

#if defined(CONFIG_INCL_SCL3300)
    InclinometerScl3300 incl(logger);
#else
    InclinometerMock incl(logger);
#endif  // CONFIG_INCL_SCL3300

    LegControlImpl leg_control;

#if defined(CONFIG_USB_DEVICE_HID)
    UsbHidZephyr usb_hid(logger);
#else
    UsbHidMock usb_hid(logger);
#endif

    // Initialize Message Handlers and Disptacher
    MessageProto msg_proto;
    MessageDispatcher dispatcher;

    // Initialize Managers
    StateManagerSml state_manager(logger);
    MessageManager msg_manager(logger, usb_hid, msg_proto, dispatcher);
    BatteryManager battery_manager(logger, battery, charger);
    ImuManager imu_manager(logger, imu);
    DisplayManager disp_manager(logger, disp);

    InclinometerManager inclino_manager(logger, incl);
    LegManager leg_manager(logger, leg_control, inclino_manager);

    // Initialize Message Handlers
    SystemMessageHandlerImpl system_msg_handler(logger);
    BatteryMessageHandlerImpl battery_msg_handler(logger, battery_manager, msg_manager);
    DisplayMessageHandlerImpl display_msg_handler(logger, disp_manager);
    InclinometerMessageHandlerImpl inclinometer_msg_handler(logger, incl);

    dispatcher.AddHandler(system_msg_handler);
    dispatcher.AddHandler(battery_msg_handler);
    dispatcher.AddHandler(display_msg_handler);
    dispatcher.AddHandler(inclinometer_msg_handler);

    state_manager.AddManager(disp_manager);
    state_manager.AddManager(msg_manager);
    state_manager.AddManager(imu_manager);
    state_manager.AddManager(battery_manager);
    state_manager.AddManager(inclino_manager);
    state_manager.AddManager(leg_manager);

    if (usb_enable(NULL) != 0) {
        logger.err("Failed to enable USB");
    }

    state_manager.Run();

    while (1) {
        k_sleep(K_MSEC(1000));
    }
    return 0;
}