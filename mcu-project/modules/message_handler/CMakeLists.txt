set(CMAKE_MODULE_PATH ${ZEPHYR_BASE}/../modules/lib/nanopb/extra)
find_package(Nanopb REQUIRED)

set(NANOPB_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}")

nanopb_generate_cpp(proto_sources proto_headers RELPATH .
    outer.proto
    google/protobuf/any.proto
)

set(MESSAGE_HANDLER_PATH "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "MESSAGE_HANDLER_PATH")

zephyr_include_directories(
    ${NANOPB_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

zephyr_library_sources(
    message_handler.cpp
    message_dispatcher.cpp
    message_encoder.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/outer.pb.c
    ${CMAKE_CURRENT_BINARY_DIR}/google/protobuf/any.pb.c
)

# Adding a custom target is a hack to make sure the .proto files are compiled before building the app
add_custom_target(message_handler ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/outer.pb.c
            ${CMAKE_CURRENT_BINARY_DIR}/google/protobuf/any.pb.c
)