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
    PRIVATE ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/managers
            ${CMAKE_SOURCE_DIR}/src/display
            ${CMAKE_SOURCE_DIR}/src/sensors
            ${CMAKE_SOURCE_DIR}/src/message_handlers
)

target_sources(app PRIVATE src/main.cpp
                           src/message_thread.cpp
                           src/logger/logger_zephyr.cpp
                           src/display/display_com35.cpp
                           src/sensors/inclinometer_impl.cpp
                           src/managers/display_manager.cpp
                           src/message_handlers/system_message_handler_impl.cpp
                           src/message_handlers/display_message_handler_impl.cpp
                           src/message_handlers/inclinometer_message_handler_impl.cpp

                           lib/usb/usb_hid.cpp
)

target_link_libraries(app PRIVATE logger)
target_link_libraries(app PRIVATE display)
target_link_libraries(app PRIVATE inclinometer)