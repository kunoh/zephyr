#include <sys/printk.h>
#include <zephyr.h>
#include <zephyr/logging/log.h>

#include "message_thread.h"
LOG_MODULE_REGISTER(main);

#define STACKSIZE 1024 * 2
#define PRIORITY 7

K_THREAD_DEFINE(message_thread_run_id, STACKSIZE, MessageThreadRun, NULL, NULL, NULL, PRIORITY, 0,
                0);

void main(void)
{
    LOG_INF("********************************");
    LOG_INF("**  TM5 IO Controller v.%s", "0.13  **");
    LOG_INF("********************************");

    return;
}