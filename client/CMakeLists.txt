cmake_minimum_required(VERSION 3.10)
project(client)
include(CTest)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add rpath settings for all executables
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN")

# Enable compilation database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Symlink compilation database to parent if needed by LSP
# execute_process(
#     COMMAND ${CMAKE_COMMAND} -E create_symlink
#     ${CMAKE_BINARY_DIR}/compile_commands.json
#     ${CMAKE_SOURCE_DIR}/compile_commands.json
# )

# Add build type parameter with DEBUG as default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# Build raylib
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/raylib)

# Create interface target for external libraries with warnings disabled
add_library(external_libs INTERFACE)
target_include_directories(external_libs INTERFACE
    ${CMAKE_SOURCE_DIR}/libs/raylib/src
    ${CMAKE_SOURCE_DIR}/libs/raygui/src
    ${CMAKE_SOURCE_DIR}/libs/rres/src
)
target_compile_options(external_libs INTERFACE -w)

# Define source files for the shared library
set(LIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/client.cpp
    ${CMAKE_SOURCE_DIR}/libs/utils_client.cpp
    ${CMAKE_SOURCE_DIR}/../libs/utils.cpp
    ${CMAKE_SOURCE_DIR}/../libs/utils_test.cpp
    ${CMAKE_SOURCE_DIR}/src/guis/main_menu.cpp
    ${CMAKE_SOURCE_DIR}/src/guis/settings_menu.cpp
)

# Define include directories
set(COMMON_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/../libs
    ${CMAKE_SOURCE_DIR}/libs
    ${CMAKE_SOURCE_DIR}/libs/raylib/examples/shaders
    ${CMAKE_SOURCE_DIR}/libs/raylib/examples/others
    ${CMAKE_SOURCE_DIR}/libs/rini/src
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/guis
)

# Steam setup
set(STEAM_DIR "${CMAKE_SOURCE_DIR}/../libs/steam/public/steam")

if(WIN32)
    set(STEAM_LIB_DIR "${CMAKE_SOURCE_DIR}/../libs/steam/redistributable_bin/win64")
    set(STEAM_LIB_FILE "${STEAM_LIB_DIR}/steam_api64.dll")
elseif(APPLE)
    set(STEAM_LIB_DIR "${CMAKE_SOURCE_DIR}/../libs/steam/redistributable_bin/osx")
    set(STEAM_LIB_FILE "${STEAM_LIB_DIR}/libsteam_api.dylib")
else()
    set(STEAM_LIB_DIR "${CMAKE_SOURCE_DIR}/../libs/steam/redistributable_bin/linux64")
    set(STEAM_LIB_FILE "${STEAM_LIB_DIR}/libsteam_api.so")
endif()

# Create shared library
add_library(client_lib SHARED ${LIB_SOURCES})

# Set library properties
set_target_properties(client_lib PROPERTIES
    OUTPUT_NAME "client"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Include directories for the library
target_include_directories(client_lib PRIVATE 
    ${COMMON_INCLUDE_DIRS}
    ${STEAM_DIR}
)

# Link libraries to the shared library
target_link_libraries(client_lib PRIVATE 
    external_libs
    raylib
    GL
    m
    pthread
    dl
    rt
    X11
    -L${STEAM_LIB_DIR}
    -lsteam_api
)

# Common compiler flags for both Debug and Release
set(COMMON_COMPILE_OPTIONS
    -Wall
    -Wextra
    -pedantic
    -Werror
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-unused-result
    -Wno-format-security
    -Wno-write-strings
    -Wno-missing-field-initializers
    -fno-gnu-unique
    -fPIC
)

# Debug-specific flags
set(DEBUG_COMPILE_OPTIONS
    -Wno-error=cpp
    -fsanitize=undefined
    -fno-sanitize-recover=all
    -O0
    -g
)

# Release-specific flags
set(RELEASE_COMPILE_OPTIONS
    -O3
    -flto
    -march=native
)

# Set compile options based on build type
target_compile_options(client_lib PRIVATE
    ${COMMON_COMPILE_OPTIONS}
    $<$<CONFIG:Debug>:${DEBUG_COMPILE_OPTIONS}>
    $<$<CONFIG:Release>:${RELEASE_COMPILE_OPTIONS}>
)

# Link UBSan only in Debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(client_lib PRIVATE ubsan)
endif()

# Add prep_models executable
add_executable(prep_models prep_models.cpp ${LIB_SOURCES})

# Set output directory for prep_models
set_target_properties(prep_models PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_include_directories(prep_models PRIVATE 
    ${COMMON_INCLUDE_DIRS}
    ${STEAM_DIR}
)
target_link_libraries(prep_models PRIVATE 
    external_libs
    raylib
    GL
    m
    pthread
    dl
    rt
    X11
    -L${STEAM_LIB_DIR}
    -lsteam_api
)
target_compile_options(prep_models PRIVATE ${COMMON_COMPILE_OPTIONS})

# Add test executable
add_executable(client_test ${CMAKE_SOURCE_DIR}/src/main_test.cpp ${LIB_SOURCES})

# Set output directory for client_test
set_target_properties(client_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_include_directories(client_test PRIVATE 
    ${COMMON_INCLUDE_DIRS}
    ${STEAM_DIR}
)

target_link_libraries(client_test PRIVATE 
    external_libs
    raylib
    GL
    m
    pthread
    dl
    rt
    X11
    -L${STEAM_LIB_DIR}
    -lsteam_api
)
target_compile_options(client_test PRIVATE ${COMMON_COMPILE_OPTIONS})

add_test(NAME ClientTests COMMAND client_test)

# Create the main executable
add_executable(client src/main.cpp)

# Set executable output directory
set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Include directories for the main executable
target_include_directories(client PRIVATE 
    ${COMMON_INCLUDE_DIRS}
    ${STEAM_DIR}
)

# Link the main executable
target_link_libraries(client PRIVATE
    external_libs
    dl
    pthread
    raylib
    GL
    m
    rt
    X11
    -L${STEAM_LIB_DIR}
    -lsteam_api
)

# Link UBSan only in Debug mode for the executable
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(client PRIVATE ubsan)
endif()

# Set compile options for the executable
target_compile_options(client PRIVATE
    ${COMMON_COMPILE_OPTIONS}
    $<$<CONFIG:Debug>:${DEBUG_COMPILE_OPTIONS}>
    $<$<CONFIG:Release>:${RELEASE_COMPILE_OPTIONS}>
)

# Copy Steam library to output directory
add_custom_command(TARGET client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${STEAM_LIB_FILE}
    ${CMAKE_SOURCE_DIR}/libsteam_api.so
)

add_dependencies(client client_lib)
add_dependencies(client_test client_lib)
add_dependencies(prep_models client_lib)

# Add custom target for running full build process
add_custom_target(run
    # First build and run prep_models
    COMMAND ${CMAKE_COMMAND} --build . --target prep_models
    COMMAND ${CMAKE_COMMAND} -E echo "Running prep_models..."
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ./prep_models
    # Then build shared library
    COMMAND ${CMAKE_COMMAND} --build . --target client_lib
    # Then build and run tests
    COMMAND ${CMAKE_COMMAND} --build . --target client_test
    COMMAND ${CMAKE_COMMAND} -E echo "Running tests..."
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ./client_test
    # Finally build and run the main executable
    COMMAND ${CMAKE_COMMAND} --build . --target client
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ./client
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running full build pipeline: prep models -> shared lib -> tests -> main executable"
)

add_custom_target(client_build_test
    # Print debug info
    COMMAND ${CMAKE_COMMAND} -E echo "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Current working directory:"
    COMMAND pwd
    # First build and run prep_models
    COMMAND ${CMAKE_COMMAND} --build . --target prep_models
    COMMAND ${CMAKE_COMMAND} -E echo "Running prep_models..."
    COMMAND ${CMAKE_COMMAND} -E echo "Looking for prep_models in:"
    COMMAND ls -la ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ./prep_models
    # Then build shared library
    COMMAND ${CMAKE_COMMAND} --build . --target client_lib
    # Then build and run tests
    COMMAND ${CMAKE_COMMAND} --build . --target client_test
    COMMAND ${CMAKE_COMMAND} -E echo "Running tests..."
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ./client_test
    # Finally build the main executable
    COMMAND ${CMAKE_COMMAND} --build . --target client
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running build pipeline: prep models -> shared lib -> tests"
)

# Add hot reload target
add_custom_target(hot
    COMMAND ${CMAKE_COMMAND} --build . --target client_lib
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Hot reloading - rebuilding shared library only"
)
