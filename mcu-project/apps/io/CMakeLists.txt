# SPDX-License-Identifier: Apache-2.0
set(overlay_file ${CMAKE_CURRENT_SOURCE_DIR}/../../boards/${BOARD}.overlay)
if(EXISTS ${overlay_file})
    list(APPEND DTC_OVERLAY_FILE "${overlay_file}")
endif()

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(io-controller)

add_subdirectory(src)

target_include_directories(app
    PRIVATE ${CMAKE_SOURCE_DIR}/src/utils
            ${CMAKE_SOURCE_DIR}/src/usb
            ${CMAKE_SOURCE_DIR}/src/logger
            ${CMAKE_SOURCE_DIR}/src/managers
            ${CMAKE_SOURCE_DIR}/src/display
            ${CMAKE_SOURCE_DIR}/src/sensors
            ${CMAKE_SOURCE_DIR}/src/message_handlers
)

target_sources(app PRIVATE src/main.cpp
                           src/logger/logger_zephyr.cpp
                           src/usb/usb_hid_zephyr.cpp
                           src/sensors/battery_mock.cpp
                           src/sensors/imu_mock.cpp
                           src/sensors/inclinometer_impl.cpp
                           src/managers/battery_manager.cpp
                           src/managers/imu_manager.cpp
                           src/managers/display_manager.cpp
                           src/managers/message_manager.cpp      
                           src/message_handlers/system_message_handler_impl.cpp
                           src/message_handlers/battery_message_handler_impl.cpp
                           src/message_handlers/display_message_handler_impl.cpp
                           src/message_handlers/inclinometer_message_handler_impl.cpp
)

target_sources_ifdef(CONFIG_BATTERY_NH2054QE34 app PRIVATE src/sensors/battery_nh2054qe34.cpp)
target_sources_ifdef(CONFIG_DISPLAY_LCD_W_FRAMEDISK app PRIVATE src/display/display_com35.cpp)
target_sources_ifdef(CONFIG_FXOS8700 app PRIVATE src/sensors/imu_fxos8700.cpp)