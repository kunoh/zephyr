#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_device.h>

#include "display_com35.h"
#include "display_manager.h"
#include "display_message_handler_impl.h"
#include "imu_fxos8700.h"
#include "imu_manager.h"
#include "imu_mock.h"
#include "inclinometer_impl.h"
#include "inclinometer_message_handler_impl.h"
#include "logger_zephyr.h"
#include "message_dispatcher.h"
#include "message_manager.h"
#include "system_message_handler_impl.h"
#include "usb_hid_zephyr.h"
#include "util.h"

LOG_MODULE_REGISTER(main);

/* Message Queues */
K_MSGQ_DEFINE(usb_hid_msgq, sizeof(MessageBuffer), 10, 1);

/* Work Items */
static k_work *usb_hid_work;

static int HandleReceiveCallback(const struct device *dev, struct usb_setup_packet *setup,
                                 int32_t *len, uint8_t **data)
{
    MessageBuffer buffer;
    memcpy(buffer.data, *data, (size_t)*len);
    buffer.length = *len;

    // For testing, to be deleted later
    if (0) {
        printk("Received bytes: \n");
        size_t i;
        for (i = 0; i < buffer.length; i++) {
            if (i > 0) printk(":");
            printk("%02X", buffer.data[i]);
        }
        printk("\n");
    }
    //

    while (k_msgq_put(&usb_hid_msgq, &buffer, K_NO_WAIT) != 0) {
        // message queue is full: purge old data & try again
        k_msgq_purge(&usb_hid_msgq);
    }

    return k_work_submit(usb_hid_work);
}

// This function is being used as an example for adding a subscriber to IMU sample data
void on_imu_data(ImuSampleData sample_data)
{
    printk("AX=%10.6f AY=%10.6f AZ=%10.6f\n", sample_data.acc.x, sample_data.acc.y,
           sample_data.acc.z);
}

int main(void)
{
    LOG_INF("********************************");
    LOG_INF("**  TM5 IO Controller v.%s", "0.13  **");
    LOG_INF("********************************");

    /* Initialize Logger */
    LoggerZephyr logger("");

    /* Initialize External Devices */
    UsbHidZephyr usb_hid(logger);
    usb_hid.SetReceiveCallback(HandleReceiveCallback);
    DisplayCOM35 disp(logger);
#if defined(CONFIG_FXOS8700)
    std::unique_ptr<Imu> imu = std::make_unique<ImuFxos8700>(logger);
#else
    std::unique_ptr<Imu> imu = std::make_unique<ImuMock>(logger);
#endif /* !CONFIG_FXOS8700 */
    InclinometerImpl incl;

    /* Initialize Message Disptacher */
    MessageProto msg_proto;
    MessageDispatcher dispatcher;

    /* Initialize Managers */
    ImuManager imu_manager(std::make_shared<LoggerZephyr>(logger), std::move(imu));
    DisplayManager disp_manager(&logger, &disp);
    MessageManager msg_manager(&logger, &usb_hid, &msg_proto, &dispatcher, &usb_hid_msgq);
    usb_hid_work = msg_manager.GetWorkItem();

    /* Initialize Message Handlers*/
    SystemMessageHandlerImpl sys_impl(logger);
    DisplayMessageHandlerImpl disp_impl(logger, disp_manager);
    InclinometerMessageHandlerImpl incl_impl(logger, incl);

    dispatcher.AddHandler(sys_impl);
    dispatcher.AddHandler(disp_impl);
    dispatcher.AddHandler(incl_impl);

    /* Playground */

    if (usb_enable(NULL) != 0) {
        logger.err("Failed to enable USB");
    }

    imu_manager.AddSubscriber(on_imu_data);
    imu_manager.StartSampling();
    disp_manager.SetBootLogo();
    disp_manager.StartSpinner();

    while (1) {
        k_sleep(K_MSEC(1000));
    }
    return 0;
}