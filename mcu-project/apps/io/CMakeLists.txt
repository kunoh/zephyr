# SPDX-License-Identifier: Apache-2.0
include(functions.cmake)

set(overlay_file ${CMAKE_CURRENT_SOURCE_DIR}/../../boards/${BOARD}.overlay)
if(EXISTS ${overlay_file})
    list(APPEND DTC_OVERLAY_FILE "${overlay_file}")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11")

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(io-controller)

set(NANOPB_OPTIONS "-I${CMAKE_SOURCE_DIR}")
nanopb_generate_cpp(proto_sources proto_headers RELPATH .
    proto/outer.proto
    proto/system.proto
    proto/display.proto
    proto/inclinometer.proto
    proto/google/protobuf/any.proto
)

find_package(PythonInterp REQUIRED)

generate_message_handler(system.proto 
                         inclinometer.proto
                         display.proto)

add_custom_target(message_handler_cpp ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/message/system_message_handler.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/message/inclinometer_message_handler.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/message/display_message_handler.cpp)

# we need to be able to include generated header files
zephyr_library_include_directories(${CMAKE_CURRENT_BINARY_DIR}
                                   ${CMAKE_CURRENT_BINARY_DIR}/message
                                   ${CMAKE_CURRENT_SOURCE_DIR}/lib/message
                                   ${CMAKE_CURRENT_SOURCE_DIR}/lib/sensors
                                   ${CMAKE_CURRENT_SOURCE_DIR}/src/message_handlers)

include_directories(${CMAKE_SOURCE_DIR})

target_sources(app PRIVATE ${proto_sources}
                            src/main.cpp
                            src/message_thread.cpp
                            src/message_handlers/system_message_handler_impl.cpp
                            src/message_handlers/display_message_handler_impl.cpp
                            src/message_handlers/inclinometer_message_handler_impl.cpp
                            lib/message/message_handler.cpp
                            lib/message/message_dispatcher.cpp
                            lib/message/message_encoder.cpp
                            lib/display/display.c
                            lib/sensors/inclinometer.cpp
                            lib/usb/usb_hid.cpp
                            ${CMAKE_CURRENT_BINARY_DIR}/message/system_message_handler.cpp
                            ${CMAKE_CURRENT_BINARY_DIR}/message/system_message_encoder.cpp
                            ${CMAKE_CURRENT_BINARY_DIR}/message/display_message_handler.cpp
                            ${CMAKE_CURRENT_BINARY_DIR}/message/display_message_encoder.cpp
                            ${CMAKE_CURRENT_BINARY_DIR}/message/inclinometer_message_handler.cpp
                            ${CMAKE_CURRENT_BINARY_DIR}/message/inclinometer_message_encoder.cpp
                           )

add_dependencies(app message_handler_cpp)