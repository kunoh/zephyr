cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(message)

add_subdirectory(${CMAKE_SOURCE_DIR}/../../apps/io/src/message_handlers ${CMAKE_CURRENT_BINARY_DIR}/message_handlers)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../apps/io/src/display ${CMAKE_CURRENT_BINARY_DIR}/display)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../apps/io/src/sensors ${CMAKE_CURRENT_BINARY_DIR}/sensors)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../apps/io/src/logger ${CMAKE_CURRENT_BINARY_DIR}/logger)

target_include_directories(app
    PRIVATE ${CMAKE_SOURCE_DIR}/../mocks/message_handlers
            ${CMAKE_SOURCE_DIR}/../mocks/display
            ${CMAKE_SOURCE_DIR}/../mocks/sensors
            ${CMAKE_SOURCE_DIR}/../mocks/logger
)

target_sources(app PRIVATE
    src/message.cpp
    ../mocks/logger/logger_mock.cpp
    ../mocks/display/display_mock.cpp
    ../mocks/sensors/inclinometer_mock.cpp
    ../mocks/message_handlers/system_message_handler_mock.cpp
    ../mocks/message_handlers/display_message_handler_mock.cpp
    ../mocks/message_handlers/inclinometer_message_handler_mock.cpp
)

target_link_libraries(app PRIVATE logger)
target_link_libraries(app PRIVATE display)
target_link_libraries(app PRIVATE inclinometer)

add_dependencies(app message_handler
                     system_target
                     display_target
                     inclinometer_target
                     )