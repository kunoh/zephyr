function(generate_message_handler)
    cmake_parse_arguments(
        ARG
        ""
        "FILENAME;DEST"
        ""
        "${ARGN}"
    )

    get_filename_component(BASENAME ${ARG_FILENAME} NAME_WE)
    add_custom_command(OUTPUT
                                # ${CMAKE_CURRENT_BINARY_DIR}/message/${BASENAME}_message_handler.cpp
                                # ${CMAKE_CURRENT_BINARY_DIR}/message/${BASENAME}_message_handler.h
                                ${CMAKE_CURRENT_BINARY_DIR}/message/${BASENAME}_message_encoder.cpp
                                ${CMAKE_CURRENT_BINARY_DIR}/message/${BASENAME}_message_encoder.h
        COMMAND "${PYTHON_EXECUTABLE}" ${MESSAGE_HANDLER_PATH}/message_generator.py ${CMAKE_CURRENT_SOURCE_DIR}/proto/${ARG_FILENAME} ${ARG_DEST} ${MESSAGE_HANDLER_PATH}/templates
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/proto/${ARG_FILENAME})

    add_custom_target(${BASENAME}_message_handler DEPENDS
                                                        #   ${ARG_DEST}/${BASENAME}_message_handler.cpp
                                                        #   ${ARG_DEST}/${BASENAME}_message_handler.h
                                                          ${ARG_DEST}/${BASENAME}_message_encoder.cpp
                                                          ${ARG_DEST}/${BASENAME}_message_encoder.h)

endfunction()