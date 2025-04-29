cmake_minimum_required(VERSION 3.11)
project(SpaceBotsClient CXX)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Raylib dependency
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(raylib)
    endif()
endif()

# Compiler flags
add_compile_options(
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
)

# Debug/Release specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -Wno-error=cpp
        -fsanitize=undefined
        -fno-sanitize-recover=all
        -O0
        -g
    )
else()
    add_compile_options(
        -O3
        -flto
        -march=native
    )
endif()

# Source files
set(COMMON_SOURCES
    src/client.cpp
    libs/utils_client.cpp
    ../libs/utils.cpp
    ../libs/utils_test.cpp
    src/guis/main_menu.cpp
    src/guis/settings_menu.cpp
)

# Steam setup
set(STEAM_SDK_PATH ${CMAKE_SOURCE_DIR}/../libs/steam)
find_library(STEAM_API_LIBRARY
    NAMES steam_api
    PATHS ${STEAM_SDK_PATH}/redistributable_bin/linux64
    NO_DEFAULT_PATH
)

# System libraries
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)

# Shared library
add_library(client SHARED ${COMMON_SOURCES})
target_include_directories(client PRIVATE
    ${CMAKE_SOURCE_DIR}/../libs
    ${CMAKE_SOURCE_DIR}/libs
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/guis
    ${STEAM_SDK_PATH}/public/steam
)
target_link_libraries(client PRIVATE
    raylib
    ${STEAM_API_LIBRARY}
    OpenGL::GL
    X11::X11
    Threads::Threads
    ${CMAKE_DL_LIBS}
    rt
)

# Set RPATH for the shared library
set_target_properties(client PROPERTIES
    SKIP_BUILD_RPATH FALSE
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN"
)

# Executables
add_executable(prep_models prep_models.cpp)
target_link_libraries(prep_models PRIVATE client)

add_executable(client_test src/main_test.cpp)
target_link_libraries(client_test PRIVATE client)

add_executable(client_exe src/main.cpp)
target_link_libraries(client_exe PRIVATE client)
set_target_properties(client_exe PROPERTIES OUTPUT_NAME client)

# Copy Steam API library
add_custom_command(
    TARGET client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${STEAM_API_LIBRARY}
    ${CMAKE_BINARY_DIR}/
)

# Platform specific settings
if (APPLE)
    target_link_libraries(client PRIVATE
        "-framework IOKit"
        "-framework Cocoa"
        "-framework OpenGL"
    )
endif()
