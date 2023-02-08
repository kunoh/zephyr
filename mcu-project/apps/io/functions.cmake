function(generate_message_handler)

    foreach(ARG_FILENAME ${ARGN})
        get_filename_component(BASENAME ${ARG_FILENAME} NAME_WE)
        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/message/${BASENAME}_message_handler.cpp
                                  ${CMAKE_CURRENT_BINARY_DIR}/message/${BASENAME}_message_handler.h
                                  ${CMAKE_CURRENT_BINARY_DIR}/message/${BASENAME}_message_encoder.cpp
                                  ${CMAKE_CURRENT_BINARY_DIR}/message/${BASENAME}_message_encoder.h
            COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/lib/message/message_generator.py" ${CMAKE_CURRENT_SOURCE_DIR}/proto/${ARG_FILENAME} ${CMAKE_CURRENT_BINARY_DIR}/message ${CMAKE_CURRENT_SOURCE_DIR}/lib/message/templates
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/proto/${ARG_FILENAME})
    endforeach()

endfunction()