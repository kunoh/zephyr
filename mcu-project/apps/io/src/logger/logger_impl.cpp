#include <zephyr/logging/log.h>
#include "logger_impl.h"

LOG_MODULE_REGISTER(logger, LOG_LEVEL_INF);

void LoggerImpl::inf(const char* msg) {
    LOG_INF("%s", msg);
}

void LoggerImpl::wrn(const char* msg) {
    LOG_WRN("%s", msg);
}

void LoggerImpl::err(const char* msg) {
    LOG_ERR("%s", msg);
}

void LoggerImpl::dbg(const char* msg) {
    LOG_DBG("%s", msg);
}