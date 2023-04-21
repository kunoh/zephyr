#pragma once
#include <zephyr/kernel.h>

template <typename T>
struct k_work_wrapper {
    T *self;
    struct k_work work;
};