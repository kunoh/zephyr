cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tests_battery_manager)

include(${CMAKE_SOURCE_DIR}/../../../CMakeLists.txt)

target_sources(app PRIVATE
                ${CMAKE_SOURCE_DIR}/src/test_battery.cpp
                ${IOC_DIR}/src/managers/battery_manager.cpp
                ${IOC_DIR}/src/sensors/battery_mock.cpp
                ${IOC_DIR}/src/sensors/battery_charger_mock.cpp
              )

target_include_directories(app PRIVATE
                            ${IOC_DIR}/src/utils
                            ${IOC_DIR}/src/managers
                            ${IOC_DIR}/src/sensors
                          )