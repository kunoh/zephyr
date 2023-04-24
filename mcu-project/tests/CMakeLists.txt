# Top level CMakeLists for include() in test directories #

get_filename_component(MCU_TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/.. ABSOLUTE)
get_filename_component(MCU_TEST_DIR ${MCU_TOP_DIR}/tests ABSOLUTE)
get_filename_component(MCU_MODULES_DIR ${MCU_TOP_DIR}/modules ABSOLUTE)
get_filename_component(IOC_DIR ${MCU_TOP_DIR}/apps/io ABSOLUTE)

message("\nUsing MCU_TOP_DIR: " ${MCU_TOP_DIR})
message("Using MCU_TEST_DIR: " ${MCU_TEST_DIR})
message("Using MCU_MODULES_DIR: " ${MCU_MODULES_DIR})
message("Using IOC_DIR: " ${IOC_DIR} "\n")