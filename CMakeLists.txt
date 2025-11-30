# Declare cmake requirements and basic configuration.
cmake_minimum_required(VERSION 3.18)
project(watchf
    VERSION 0.1.0
    DESCRIPTION "Watch for file changes"
    HOMEPAGE_URL "https://github.com/markdlehane/watchf"
    LANGUAGES C
)

# Configure C.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Force the output directory to be /build. This prevents main
# from being placed in a subdirectory.
#
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(UPDATE_TOOL "${CMAKE_SOURCE_DIR}/update_version.py")

# Define well known names for the build modes.
set(BUILD_FOR_DEBUG "Debug")
set(BUILD_FOR_RELEASE "Release")
set(BUILD_FOR_TESTING "Testing")

# Select a build mode: Debug, Release and Test.
set(TARGET_BUILD ${BUILD_FOR_RELEASE} CACHE STRING "Build target")

# Build according to the target type.
if(TARGET_BUILD STREQUAL ${BUILD_FOR_RELEASE} OR TARGET_BUILD STREQUAL ${BUILD_FOR_DEBUG})
    message(STATUS "${TARGET_BUILD} build with main.c")
    add_subdirectory(main)

elseif(TARGET_BUILD STREQUAL ${BUILD_FOR_TESTING})
    message(STATUS "Testing build with testing.c")

    # Include CTest and enable test before including unity etc.
    include(CTest)
    enable_testing()

    # Add unity and the tests.
    add_subdirectory(unity)
    add_subdirectory(test)
    
else()
    message("YOU MUST CHOOSE A VALID BUILD TYPE")

endif()

# End.
