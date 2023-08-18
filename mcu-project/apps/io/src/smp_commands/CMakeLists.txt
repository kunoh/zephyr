# include(functions.cmake)
find_package(PythonInterp REQUIRED)

set(CMAKE_MODULE_PATH ${ZEPHYR_BASE}/../modules/lib/nanopb/extra)
find_package(Nanopb REQUIRED)
set(NANOPB_OPTIONS "-I${CMAKE_SOURCE_DIR}")

set(BASENAMES "system_new")

foreach(BASENAME ${BASENAMES})
    nanopb_generate_cpp(proto_sources proto_headers RELPATH proto
        proto/${BASENAME}.proto
    )

    # generate_message_handler(FILENAME ${BASENAME}.proto
    #                         DEST ${CMAKE_CURRENT_BINARY_DIR}/message
    #                         )

    zephyr_include_directories(
        #${CMAKE_CURRENT_SOURCE_DIR}
                            ${CMAKE_CURRENT_BINARY_DIR}
                            #${CMAKE_CURRENT_BINARY_DIR}/message
    )


    zephyr_library_sources(
        ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.pb.c
    )

    # Adding a custom target is a hack to make sure the .proto files are compiled before building the app
    add_custom_target(${BASENAME}_target_new ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.pb.c
    )
endforeach()