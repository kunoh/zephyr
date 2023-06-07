#pragma once
#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>

template <typename T>
struct k_work_wrapper {
    T *self;
    k_work work;
};

template <typename T, typename P>
struct self_wrapper {
    T *self;
    P p;
};
