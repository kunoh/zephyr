cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tests_message)

include(${CMAKE_SOURCE_DIR}/../../../CMakeLists.txt)

add_subdirectory(${IOC_DIR}/src/message_handlers ${CMAKE_CURRENT_BINARY_DIR}/message_handlers)

target_include_directories(app PRIVATE
            ${IOC_DIR}/src/utils
            ${IOC_DIR}/src/display
            ${IOC_DIR}/src/managers
            ${IOC_DIR}/src/message_handlers
)

target_sources(app PRIVATE
            src/test_dispatcher_chain.cpp
            ${IOC_DIR}/src/display/display_mock.cpp
            ${IOC_DIR}/src/managers/display_manager.cpp
            ${IOC_DIR}/src/message_handlers/system_message_handler_impl.cpp
            ${IOC_DIR}/src/message_handlers/display_message_handler_impl.cpp
)
