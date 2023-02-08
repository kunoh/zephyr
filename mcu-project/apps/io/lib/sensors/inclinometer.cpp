#include "inclinometer.h"

LOG_MODULE_REGISTER(inclinometer, LOG_LEVEL_INF);

bool Inclinometer::write() {
    LOG_INF("Writing to inclinometer");
    return true;
}