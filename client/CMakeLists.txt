cmake_minimum_required(VERSION 3.10)
project(client)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add rpath settings for all executables
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN")

# Enable compilation database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add build type parameter with DEBUG as default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# Set up sanitizer configuration globally, but only for our code
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Create a function to set sanitizer flags for our targets
    function(target_set_sanitizers target_name)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${target_name} PRIVATE -fsanitize=undefined -fno-sanitize-recover=all)
            target_link_libraries(${target_name} PRIVATE ubsan)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${target_name} PRIVATE -fsanitize=undefined)
            target_link_options(${target_name} PRIVATE -fsanitize=undefined)
        endif()
    endfunction()
endif()

# Set OpenGL preference to GLVND
if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

# Platform options with defaults
if(UNIX AND NOT APPLE)
    # Linux-specific setup
    option(USE_WAYLAND "Build with Wayland support" OFF)
    option(USE_X11 "Build with X11 support" ON)

    # Check for required packages based on options
    if(USE_WAYLAND)
        message(STATUS "Checking Wayland dependencies...")
        
        # Try to find pkg-config first
        find_package(PkgConfig)
        if(NOT PKG_CONFIG_FOUND)
            message(STATUS "pkg-config not found - required for Wayland support")
            message(STATUS "On Nix, add pkgconfig to your development environment")
            set(USE_WAYLAND OFF)
            set(USE_X11 ON)
        else()
            # If pkg-config is found, check for Wayland
            pkg_check_modules(WAYLAND wayland-client wayland-protocols wayland-egl wayland-cursor xkbcommon libffi)
            find_program(WAYLAND_SCANNER wayland-scanner)
            
            if(NOT WAYLAND_FOUND)
                message(STATUS "Wayland libraries not found")
                set(USE_WAYLAND OFF)
                set(USE_X11 ON)
            else()
                link_directories(${WAYLAND_LIBRARY_DIRS})
                include_directories(${WAYLAND_INCLUDE_DIRS})
            endif()
            
            if(NOT WAYLAND_SCANNER)
                message(STATUS "wayland-scanner not found")
                set(USE_WAYLAND OFF)
                set(USE_X11 ON)
            endif()
        endif()
    endif()

    if(USE_X11)
        find_package(X11)
        if(NOT X11_FOUND)
            message(STATUS "X11 not found - building without X11 support")
            set(USE_X11 OFF)
        endif()
    endif()

    find_package(OpenGL)
    find_package(Threads)
endif()

# Build raylib with platform-appropriate settings
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# set(PLATFORM "Desktop" CACHE STRING "Platform to build for." FORCE)
set(PLATFORM "Desktop" CACHE STRING "Platform to build for." FORCE)
set(USE_EXTERNAL_GLFW "OFF" CACHE STRING "Link raylib against system GLFW instead of embedded one" FORCE)

# Configure GLFW based on platform
if(UNIX AND NOT APPLE)
    set(GLFW_BUILD_WAYLAND ${USE_WAYLAND} CACHE BOOL "Build GLFW with Wayland support" FORCE)
    set(GLFW_BUILD_X11 ${USE_X11} CACHE BOOL "Build GLFW with X11 support" FORCE)
else()
    # Windows and MacOS don't need Wayland or X11
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build GLFW with Wayland support" FORCE)
    set(GLFW_BUILD_X11 OFF CACHE BOOL "Build GLFW with X11 support" FORCE)
endif()

# Common raylib settings
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
set(OPENGL_VERSION "2.1" CACHE STRING "" FORCE)

# Add raylib
add_subdirectory(libs/raylib)

# Create interface target for external libraries with warnings disabled
add_library(external_libs INTERFACE)
target_include_directories(external_libs SYSTEM INTERFACE
    ${CMAKE_SOURCE_DIR}/libs/raylib/src
    ${CMAKE_SOURCE_DIR}/libs/raygui/src
    ${CMAKE_SOURCE_DIR}/libs/rres/src
    ${CMAKE_SOURCE_DIR}/libs/raylib/examples/shaders
    ${CMAKE_SOURCE_DIR}/libs/raylib/examples/others
    ${CMAKE_SOURCE_DIR}/libs/rini/src
)

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
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/guis
)

# Steam setup
set(STEAM_DIR "${CMAKE_SOURCE_DIR}/../libs/steam/public/steam")

if(WIN32)
    set(STEAM_LIB_DIR "${CMAKE_SOURCE_DIR}/../libs/steam/redistributable_bin/win64")
    set(STEAM_LIB_FILE "${STEAM_LIB_DIR}/steam_api64.dll")
    set(LIB_EXTENSION "dll")
elseif(APPLE)
    set(STEAM_LIB_DIR "${CMAKE_SOURCE_DIR}/../libs/steam/redistributable_bin/osx")
    set(STEAM_LIB_FILE "${STEAM_LIB_DIR}/libsteam_api.dylib")
    set(LIB_EXTENSION "dylib")
else()
    set(STEAM_LIB_DIR "${CMAKE_SOURCE_DIR}/../libs/steam/redistributable_bin/linux64")
    set(STEAM_LIB_FILE "${STEAM_LIB_DIR}/libsteam_api.so")
    set(LIB_EXTENSION "so")
endif()

# Create shared library
add_library(client_lib SHARED ${LIB_SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_set_sanitizers(client_lib)
endif()

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
    ${OPENGL_LIBRARIES}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(client_lib PRIVATE
        m
        pthread
        dl
        rt
        X11
    )
endif()

target_link_libraries(client_lib PRIVATE
    -L${STEAM_LIB_DIR}
    -lsteam_api
)

# Common compiler flags for both Debug and Release
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMMON_COMPILE_OPTIONS
        /W4               # Warning level 4
        /WX               # Treat warnings as errors
        /wd4100           # Unreferenced formal parameter
        /wd4189           # Local variable initialized but not referenced
        /wd4996           # Unsafe function warnings
        /wd4505           # Unreferenced local function removed (similar to -Wno-unused-result)
        /wd4706           # Assignment within conditional expression
        /wd4127           # Conditional expression is constant
        /wd4200           # Zero-sized array in struct/union (similar to -Wno-missing-field-initializers)
        /MP               # Multi-processor compilation
        /permissive-      # Enables GNU-like behavior
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(COMMON_COMPILE_OPTIONS
        -Wall
        -Wextra
        -pedantic
        -Werror
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-format-security
        -Wno-missing-field-initializers
        # -Wstrict-aliasing=3
        # -fstrict-aliasing
        # -Wcast-align
        # -Wcast-qual
        # -Wpointer-arith
        # -Wconversion
        # -Wsign-conversion
        # -Wfloat-conversion
        # -Wstrict-overflow=5
        # -Wdouble-promotion
        # -Wold-style-cast
        # -Wuseless-cast
        # -Wuninitialized
        # -Wmaybe-uninitialized
        -fPIC
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND COMMON_COMPILE_OPTIONS
            -Wno-unused-but-set-variable
            -Wno-unused-result
            -Wno-write-strings
            -fno-gnu-unique
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        list(APPEND COMMON_COMPILE_OPTIONS
            -fgnu-keywords
            -fgnu89-inline
            -fpermissive
            -Wno-gnu
            -Wno-gnu-zero-variadic-macro-arguments
            -Wno-writable-strings
        )
    endif()
endif()

# Debug-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(DEBUG_COMPILE_OPTIONS
        /Od     # Disable optimization
        /Zi     # Generate debug info
        /RTC1   # Runtime error checks
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(DEBUG_COMPILE_OPTIONS
        -O0
        -g
        -fsanitize=undefined
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND DEBUG_COMPILE_OPTIONS
            -Wno-error=cpp
            -fno-sanitize-recover=all
        )
    endif()
endif()

# Release-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(RELEASE_COMPILE_OPTIONS
        /O2     # Maximum optimization
        /GL     # Whole program optimization
        /Gy     # Function-level linking
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(RELEASE_COMPILE_OPTIONS
        -O3
        -flto
        -march=native
    )
endif()

# Set compile options based on build type
target_compile_options(client_lib PRIVATE
    ${COMMON_COMPILE_OPTIONS}
    $<$<CONFIG:Debug>:${DEBUG_COMPILE_OPTIONS}>
    $<$<CONFIG:Release>:${RELEASE_COMPILE_OPTIONS}>
)

# Add prep_models executable
add_executable(prep_models prep_models.cpp ${LIB_SOURCES})

# Set rrespacker paths and commands based on platform
if(WIN32)
    set(RRESPACKER_PATH "${CMAKE_SOURCE_DIR}/libs/rrespacker/rrespacker.exe")
    set(RESOURCE_CMD "%s -o resources.rres --rrp resources.rrp")
elseif(APPLE)
    set(RRESPACKER_PATH "${CMAKE_SOURCE_DIR}/libs/rrespacker/rrespacker.app/Contents/MacOS/rrespacker")
    set(RESOURCE_CMD "%s -o resources.rres --rrp resources.rrp")
else() # Linux
    set(RRESPACKER_PATH "${CMAKE_SOURCE_DIR}/libs/rrespacker/rrespacker")
    set(RESOURCE_CMD "%s -o resources.rres --rrp resources.rrp")
endif()

target_compile_definitions(prep_models PRIVATE 
    RRESPACKER_PATH="${RRESPACKER_PATH}"
    RESOURCE_CMD="${RESOURCE_CMD}"
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_set_sanitizers(prep_models)
endif()

# Set output directory for prep_models
set_target_properties(prep_models PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_include_directories(prep_models PRIVATE 
    ${COMMON_INCLUDE_DIRS}
    ${STEAM_DIR}
)

# Link libraries
target_link_libraries(prep_models PRIVATE 
    external_libs
    raylib
    ${OPENGL_LIBRARIES}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(prep_models PRIVATE
        m
        pthread
        dl
        rt
        X11
    )
endif()

target_link_libraries(prep_models PRIVATE
    -L${STEAM_LIB_DIR}
    -lsteam_api
)

# Set compile options based on build type
target_compile_options(prep_models PRIVATE
    ${COMMON_COMPILE_OPTIONS}
    $<$<CONFIG:Debug>:${DEBUG_COMPILE_OPTIONS}>
    $<$<CONFIG:Release>:${RELEASE_COMPILE_OPTIONS}>
)

# Add test executable
add_executable(client_test ${CMAKE_SOURCE_DIR}/src/main_test.cpp ${LIB_SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_set_sanitizers(client_test)
endif()

# Set output directory for client_test
set_target_properties(client_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_include_directories(client_test PRIVATE 
    ${COMMON_INCLUDE_DIRS}
    ${STEAM_DIR}
)

# Link libraries to the shared library
target_link_libraries(client_test PRIVATE 
    external_libs
    raylib
    ${OPENGL_LIBRARIES}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(client_test PRIVATE
        m
        pthread
        dl
        rt
        X11
    )
endif()

target_link_libraries(client_test PRIVATE
    -L${STEAM_LIB_DIR}
    -lsteam_api
)

# Set compile options based on build type
target_compile_options(client_test PRIVATE
    ${COMMON_COMPILE_OPTIONS}
    $<$<CONFIG:Debug>:${DEBUG_COMPILE_OPTIONS}>
    $<$<CONFIG:Release>:${RELEASE_COMPILE_OPTIONS}>
)

target_compile_definitions(client_test PRIVATE 
    CLIENT_LIB_PATH="./libclient.${LIB_EXTENSION}"
)

# Create the main executable
add_executable(client src/main.cpp)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_set_sanitizers(client)
endif()

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
    raylib
    ${OPENGL_LIBRARIES}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(client PRIVATE
        m
        pthread
        dl
        rt
        X11
    )
endif()

target_link_libraries(client PRIVATE
    -L${STEAM_LIB_DIR}
    -lsteam_api
)

# Set compile options based on build type
target_compile_options(client PRIVATE
    ${COMMON_COMPILE_OPTIONS}
    $<$<CONFIG:Debug>:${DEBUG_COMPILE_OPTIONS}>
    $<$<CONFIG:Release>:${RELEASE_COMPILE_OPTIONS}>
)

target_compile_definitions(client PRIVATE 
    CLIENT_LIB_PATH="./libclient.${LIB_EXTENSION}"
)

# Copy Steam library to output directory
add_custom_command(TARGET client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${STEAM_LIB_FILE}
    ${CMAKE_SOURCE_DIR}/libsteam_api.${LIB_EXTENSION}
)

add_dependencies(client client_lib)
add_dependencies(client_test client_lib)
