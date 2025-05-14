cmake_minimum_required(VERSION 3.10)
project(SpaceBots VERSION 1.0)

# Set C++ standard globally
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Debug build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# Platform options with defaults
option(USE_WAYLAND "Build with Wayland support" OFF)
option(USE_X11 "Build with X11 support" ON)

# Enable testing for the whole project
enable_testing()

# Platform-specific setup
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(Threads REQUIRED)
endif()

# Configure and build client
execute_process(
    COMMAND ${CMAKE_COMMAND} 
        -S "${CMAKE_SOURCE_DIR}/client"
        -B "${CMAKE_SOURCE_DIR}/client/build"
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DUSE_WAYLAND=${USE_WAYLAND}
        -DUSE_X11=${USE_X11}
)

# Configure and build server
execute_process(
    COMMAND ${CMAKE_COMMAND}
        -S "${CMAKE_SOURCE_DIR}/server"
        -B "${CMAKE_SOURCE_DIR}/server/build"
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

# Add a target to build and test everything
add_custom_target(full_check
    COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/client/build" --target client_build_test
    COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/server/build" --target server_build_test
    COMMENT "Building and testing everything"
)
