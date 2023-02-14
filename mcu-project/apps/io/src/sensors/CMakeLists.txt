add_library(display INTERFACE)
target_include_directories(display INTERFACE .)

add_library(inclinometer INTERFACE)
target_include_directories(inclinometer INTERFACE .)
#add_library(sensors OBJECT
#    display.cpp
#    inclinometer.cpp
#)
#
#target_include_directories(sensors
#    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
#)
#