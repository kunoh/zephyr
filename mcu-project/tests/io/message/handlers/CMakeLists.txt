cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(message_handler_tests)

include(${CMAKE_SOURCE_DIR}/../../../CMakeLists.txt)

add_subdirectory(${IOC_DIR}/src/message_handlers ${CMAKE_CURRENT_BINARY_DIR}/message_handlers)

target_include_directories(app PRIVATE
                           ${IOC_DIR}/src/logger
                           ${IOC_DIR}/src/managers
                           ${IOC_DIR}/src/message_handlers
                           ${IOC_DIR}/src/sensors
                           ${IOC_DIR}/src/utils
                           ${IOC_DIR}/src/usb
                          )

target_sources(app PRIVATE
               ${CMAKE_SOURCE_DIR}/src/test_handlers.cpp
               ${IOC_DIR}/src/logger/logger_mock.cpp
               ${IOC_DIR}/src/usb/usb_hid_mock.cpp
               ${IOC_DIR}/src/sensors/battery_mock.cpp
               ${IOC_DIR}/src/sensors/battery_charger_mock.cpp
               ${IOC_DIR}/src/managers/battery_manager.cpp
               ${IOC_DIR}/src/managers/message_manager.cpp
               ${IOC_DIR}/src/message_handlers/battery_message_handler_impl.cpp
              )
