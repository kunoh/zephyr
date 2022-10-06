/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include "message_thread.h"

/* size of stack area used by each thread */
#define STACKSIZE 2048

/* scheduling priority used by each thread */
#define PRIORITY 7

K_THREAD_DEFINE(message_thread_run_id, STACKSIZE, MessageThreadRun, NULL, NULL, NULL,
		PRIORITY, 0, 0);

void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
}