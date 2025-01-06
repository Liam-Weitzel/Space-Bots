#!/bin/bash

# Exit on error
set -e

# Function for red echo messages
print_status() {
    echo -e "\e[31m$1\e[0m"
}

build_shared_lib() {
    print_status "Building shared library..."
    TIMESTAMP=$(date +%s)
    TEMP_NAME="libserver_${TIMESTAMP}.so"
    FINAL_NAME="libserver.so"
    
    # Build to a temporary file first
    g++ -shared -fPIC $SRC_DIR/server.cpp \
        $INCLUDES \
        $STEAM_LIBS \
        $SYSTEM_LIBS \
        $COMPILER_FLAGS \
        -o ./$TEMP_NAME

    # Move temp file to final name
    mv ./$TEMP_NAME ./$FINAL_NAME
    sync
}

LIBS_DIR="./libs"
COMMON_LIBS_DIR="../libs"
ENTT_DIR="$COMMON_LIBS_DIR/entt"
STEAM_DIR="$COMMON_LIBS_DIR/steam/public/steam"
STEAM_LIB_DIR="$COMMON_LIBS_DIR/steam/redistributable_bin/linux64"
STEAM_LIB_FILE="$STEAM_LIB_DIR/libsteam_api.so"
SRC_DIR="./src"

# Common flags
INCLUDES=" -I $ENTT_DIR/src/entt -I $SRC_DIR -I $STEAM_DIR"
STEAM_LIBS=" -l steam_api -L $STEAM_LIB_DIR"
SYSTEM_LIBS=" -lm -lpthread -ldl -lrt"
COMPILER_FLAGS=" -Wl,-rpath,\$ORIGIN/ -fno-gnu-unique -Wno-format-security -g"

# Check if we're doing a hot reload build
if [ "$1" = "hot" ]; then
    build_shared_lib
    print_status "Hot reload build complete!"
    exit 0
fi

# Copy steamworks library
cp $STEAM_LIB_FILE ./

# Build the server
build_shared_lib

print_status "Building main executable..."
g++ $SRC_DIR/main.cpp \
    $INCLUDES \
    $STEAM_LIBS \
    $SYSTEM_LIBS \
    $COMPILER_FLAGS \
    -o ./server

# Generate compilation database only if it doesn't exist
if [ ! -f "compile_commands.json" ]; then
    print_status "Generating compilation database..."
    bear -- g++ -shared -fPIC $SRC_DIR/server.cpp \
        $INCLUDES \
        $STEAM_LIBS \
        $SYSTEM_LIBS \
        $COMPILER_FLAGS \
        -o ./libserver.so

    bear -- g++ $SRC_DIR/main.cpp \
        $INCLUDES \
        $STEAM_LIBS \
        $SYSTEM_LIBS \
        $COMPILER_FLAGS \
        -o ./server
fi

print_status "Build complete!"

# Run the server
./server
