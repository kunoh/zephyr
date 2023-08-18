#pragma once
/**
 * Trackman Groups for SMP
 * User groups start from 64, see doc
 * https://docs.zephyrproject.org/latest/services/device_mgmt/smp_protocol.html#management-group-id-s
 * <zephyr/mgmt/mcumgr/mgmt/mgmt.h> is not included to make _this_ header compatible with clients
 * and therefor 64 is hardcoded instead of using MGMT_GROUP_ID_PERUSER
 */
enum tm_mcumgr_group_t {
    // SYSTEM
    TM_MGMT_GROUP_ID_SYSTEM = 64,

    // DISPLAY (Not used atm, just for illustration)
    TM_MGMT_GROUP_ID_DISPLAY,
};

/**
 * Command IDs for System management group
 */
enum system_mgmt_commands {
    // Hello world using PB (used as an illustration of usage)
    SYSTEM_MGMT_ID_HELLO_WORLD = 0,

    // Get reset cause
    SYSTEM_MGMT_ID_RESET_CAUSE,

    // It is used to allocate enough space for all commands. Should be increased everytime a new
    // command is added
    SYSTEM_NUMBER_OF_COMMANDS
};

/**
 * Command IDs for Display management group
 */
enum display_mgmt_commands {
    // Hello world using Cbor
    DISPLAY_MGMT_ID_NEXT_FRAME = 0,

    // Hello world using PB (Will not be implemented, just used as an illustration of usage)
    DISPLAY_MGMT_ID_STOP_SPINNER = 1,

    // It is used to allocate enough space for all commands. Should be increased everytime a new
    // command is added
    DISPLAY_NUMBER_OF_COMMANDS
};