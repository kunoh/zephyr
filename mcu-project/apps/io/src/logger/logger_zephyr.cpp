#include <zephyr/logging/log.h>
#include "logger_zephyr.h"

LOG_MODULE_REGISTER(logger, LOG_LEVEL_INF);

void LoggerZephyr::inf(const char* msg) {
    LOG_INF("%s", msg);
}

void LoggerZephyr::wrn(const char* msg) {
    LOG_WRN("%s", msg);
}

void LoggerZephyr::err(const char* msg) {
    LOG_ERR("%s", msg);
}

void LoggerZephyr::dbg(const char* msg) {
    LOG_DBG("%s", msg);
}