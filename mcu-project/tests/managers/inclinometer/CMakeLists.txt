cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tests_inclino_manager)

include(${CMAKE_SOURCE_DIR}/../../CMakeLists.txt)

target_sources(app PRIVATE
                ${CMAKE_SOURCE_DIR}/src/test_inclino.cpp
                ${IOC_DIR}/src/logger/logger_zephyr.cpp
                ${IOC_DIR}/src/managers/inclinometer_manager.cpp
                ${IOC_DIR}/src/sensors/inclinometer_mock.cpp
              )

target_include_directories(app PRIVATE
                            ${IOC_DIR}/src/utils
                            ${IOC_DIR}/src/managers
                            ${IOC_DIR}/src/sensors
                            ${IOC_DIR}/src/logger
                          )