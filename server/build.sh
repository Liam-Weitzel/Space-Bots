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
ENTT_DIR="$LIBS_DIR/entt"
STEAM_DIR="$LIBS_DIR/steam"
STEAM_LIB_DIR="$STEAM_DIR/linux64"
SRC_DIR="./src"

# Common flags
INCLUDES="-I $LIBS_DIR -I $ENTT_DIR/src/entt -I $SRC_DIR -I $STEAM_DIR"
STEAM_LIBS=" -l steam_api -L $STEAM_LIB_DIR"
SYSTEM_LIBS="-lGL -lm -lpthread -ldl -lrt -lX11"
COMPILER_FLAGS="-Wl,-rpath,\$ORIGIN/ -fno-gnu-unique -Wno-format-security -g"

# Check if we're doing a hot reload build
if [ "$1" = "hot" ]; then
    build_shared_lib
    print_status "Hot reload build complete!"
    exit 0
fi

# Regular full build path continues here
mkdir -p $LIBS_DIR

if [ ! -d "$ENTT_DIR" ]; then
    print_status "Cloning ENTT..."
    git clone https://github.com/skypjack/entt.git $ENTT_DIR
    cd $ENTT_DIR
    git reset --hard 9a19f9aa2f5c7d822ceeea06bce1b0a882c2cb97
    cd ..
fi

# Copy steamworks library
cp $STEAM_LIB_DIR/libsteam_api.so ./

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
