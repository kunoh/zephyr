set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(message)

set(NANOPB_OPTIONS "-I${CMAKE_SOURCE_DIR}")
nanopb_generate_cpp(proto_sources proto_headers RELPATH .
    proto/message.proto
    proto/google/protobuf/any.proto
)

find_package(PythonInterp REQUIRED)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/message/message_handler.cpp 
                          ${CMAKE_CURRENT_BINARY_DIR}/message/message_handler.hpp
                          ${CMAKE_CURRENT_BINARY_DIR}/message/message_encoder.cpp
                          ${CMAKE_CURRENT_BINARY_DIR}/message/message_encoder.hpp
    COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../../apps/io/lib/message/message_generator.py" ${CMAKE_SOURCE_DIR}/../../apps/io/proto/message.proto ${CMAKE_CURRENT_BINARY_DIR}/message ${CMAKE_SOURCE_DIR}/../../apps/io/lib/message/templates
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../../apps/io/proto/message.proto)

add_custom_target(message_handler_cpp ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/message/message_handler.cpp)

# we need to be able to include generated header files
zephyr_library_include_directories(${CMAKE_CURRENT_BINARY_DIR}
                                   ${CMAKE_CURRENT_BINARY_DIR}/message)

include_directories(${CMAKE_SOURCE_DIR}/../../apps/io)

FILE(GLOB app_sources src/*.cpp src/display_mock.c
                      ${CMAKE_SOURCE_DIR}/../../apps/io/src/message_type_handler.cpp
                      )

target_sources(app PRIVATE
  ${proto_sources}
  ${app_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/message/message_handler.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/message/message_encoder.cpp
  )

add_dependencies(app message_handler_cpp)