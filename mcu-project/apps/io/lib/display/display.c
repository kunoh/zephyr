/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(display, LOG_LEVEL_INF);

#include <zephyr.h>
#include <zephyr/device.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/posix/poll.h>

#ifdef CONFIG_ARCH_POSIX
#include "posix_board_if.h"
#define RETURN_FROM_MAIN(exit_code) posix_exit_main(exit_code)
static void posix_exit_main(int exit_code)
{
#if CONFIG_TEST
	if (exit_code == 0) {
		LOG_INF("PROJECT EXECUTION SUCCESSFUL");
	} else {
		LOG_INF("PROJECT EXECUTION FAILED");
	}
#endif
	posix_exit(exit_code);
}
#else
#define RETURN_FROM_MAIN(exit_code) return
#endif

extern void display_lcd_next_framebuffer_ready(const struct device *dev);
extern void display_stop_spinner(void);

static struct k_poll_signal new_frame_signal;
static const struct device *display_dev;

// Graphic Resources
const uint8_t logo[] = {
    #include "TrackManLogo.h"
};

const uint8_t spinner[] = {
    #include "Circle1.h"
	#include "Circle2.h"
	#include "Circle3.h"
	#include "Circle4.h"
	#include "Circle5.h"
	#include "Circle6.h"
	#include "Circle7.h"
	#include "Circle8.h"
};

// static void display_fill_buffer_rgb888(uint32_t color, uint8_t *buf, size_t buf_size)
// {
// 	for (size_t idx = 0; idx < buf_size; idx += 3) {
// 		*(buf + idx + 0) = color >> 16;
// 		*(buf + idx + 1) = color >> 8;
// 		*(buf + idx + 2) = color >> 0;
// 	}
// }

// static void display_show_rect(uint32_t x, uint32_t y, uint32_t color) {
// 	size_t rect_w = 20;
// 	size_t rect_h = 30;
// 	size_t buf_size = rect_w * rect_h * 3;
// 	uint8_t *buf = k_malloc(buf_size);

// 	struct display_buffer_descriptor buf_desc = {.pitch = rect_w, .width = rect_w, .height = rect_h};
// 	display_fill_buffer_rgb888(color, buf, buf_size);
// 	display_write(display_dev, x, y, &buf_desc, buf);
// }

void display_signal_new_frame(void)
{
	k_poll_signal_raise(&new_frame_signal, 0x1313);
}

void my_work_handler(struct k_work *work)
{
    static uint32_t spinner_idx=0;

	uint8_t *spin = (uint8_t*)spinner +  spinner_idx%8 * (64*64*3);
	struct display_buffer_descriptor buf_desc = { .width = 64, .height = 64, .pitch = 64 };
	display_write(display_dev, (320-64)/2, 90, &buf_desc, spin);
	spinner_idx++;
}

K_WORK_DEFINE(my_work, my_work_handler);

void my_timer_handler(struct k_timer *dummy)
{
    k_work_submit(&my_work);
}

K_TIMER_DEFINE(my_timer, my_timer_handler, NULL);


void display_stop_spinner(void)
{
	k_timer_stop(&my_timer);
	LOG_INF("Got stop spinner!");
}

void display_run(void)
{
	//Start usb to enable ramdisk block device
	int ret = usb_enable(NULL);
	if (ret != 0 && ret != -EALREADY ) {
		LOG_ERR("Failed to enable MSC USB");
		return;
	}

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device %s not found. Aborting...", display_dev->name);
		RETURN_FROM_MAIN(1);
	}

	struct display_buffer_descriptor buf_desc = { .width = 198, .height = 16, .pitch = 198 };
	display_write(display_dev, (320-198)/2, 10, &buf_desc, logo);

	k_timer_start(&my_timer, K_MSEC(50), K_MSEC(200));


    k_poll_signal_init(&new_frame_signal);
    struct k_poll_event events[1] = {
        K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &new_frame_signal)
    };

    while (true) {
        k_poll(events, 1, K_FOREVER);

        if (events[0].signal->result == 0x1313) {
			LOG_INF("Got new frame!");
			display_lcd_next_framebuffer_ready(display_dev);
        } else {
			LOG_ERR("Weird signal error!");
		}

        events[0].signal->signaled = 0;
        events[0].state = K_POLL_STATE_NOT_READY;
    }
	
}