# SPDX-License-Identifier: Apache-2.0

set(overlay_file ${CMAKE_CURRENT_SOURCE_DIR}/../../boards/${BOARD}.overlay)
if(EXISTS ${overlay_file})
    list(APPEND DTC_OVERLAY_FILE "${overlay_file}")
endif()
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

target_sources(app PRIVATE src/main.c)
