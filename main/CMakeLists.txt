# Define the main library.

message("** Included Release | Debug entry point.")

# Add the base executable.
add_executable(
    ${PROJECT_NAME} 
    main.c
    watch.c
)

# Add a custom command to update a version number before each build.
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD 
    COMMAND python3 ${UPDATE_TOOL} main.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

# End.
