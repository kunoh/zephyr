#include "display.h"

LOG_MODULE_REGISTER(display, LOG_LEVEL_INF);

bool Display::write() {
    LOG_INF("Writing to Display");
    return true;
}