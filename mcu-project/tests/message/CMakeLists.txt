include(functions.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(message)

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
                                   ${CMAKE_SOURCE_DIR}/../../apps/io/lib/message
                                   ${CMAKE_SOURCE_DIR}/../../apps/io/lib/sensors
                                   ${CMAKE_SOURCE_DIR}/../../apps/io/src/message_handlers)

include_directories(${CMAKE_SOURCE_DIR}/../../apps/io)

FILE(GLOB app_sources src/message.cpp
                      ../mocks/display_mock.c
                      ../mocks/display.cpp
                      ../mocks/system_message_handler_impl.cpp
                      ../mocks/display_message_handler_impl.cpp
                      ../mocks/inclinometer_message_handler_impl.cpp
                      )

target_sources(app PRIVATE
    ${proto_sources}
    ${app_sources}
    ${CMAKE_SOURCE_DIR}/../../apps/io/lib/message/message_dispatcher.cpp
    ${CMAKE_SOURCE_DIR}/../../apps/io/lib/message/message_encoder.cpp
    ${CMAKE_SOURCE_DIR}/../../apps/io/lib/message/message_handler.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/message/system_message_handler.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/message/system_message_encoder.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/message/display_message_handler.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/message/display_message_encoder.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/message/inclinometer_message_handler.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/message/inclinometer_message_encoder.cpp
    )

add_dependencies(app message_handler_cpp)