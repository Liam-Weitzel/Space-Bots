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
    TEMP_NAME="libgame_${TIMESTAMP}.so"
    FINAL_NAME="libgame.so"
    
    # Build to a temporary file first
    g++ -shared -fPIC $SRC_DIR/game.cpp \
        $INCLUDES \
        $RAYLIB_LIBS \
        $SYSTEM_LIBS \
        $COMPILER_FLAGS \
        -o $BUILD_DIR/$TEMP_NAME

    # Move temp file to final name
    mv $BUILD_DIR/$TEMP_NAME $BUILD_DIR/$FINAL_NAME
    sync
}

LIBS_DIR="./libs"
RAYLIB_DIR="$LIBS_DIR/raylib"
RAYLIB_SRC="$RAYLIB_DIR/src"
ENTT_DIR="$LIBS_DIR/entt"
SRC_DIR="./src"
BUILD_DIR="./build"

# Common flags
INCLUDES="-I $RAYLIB_SRC -I $LIBS_DIR -I $ENTT_DIR/src/entt -I $SRC_DIR"
RAYLIB_LIBS="-L $RAYLIB_SRC -L $RAYLIB_SRC/rtext -lraylib"
SYSTEM_LIBS="-lGL -lm -lpthread -ldl -lrt -lX11"
COMPILER_FLAGS="-fno-gnu-unique -g"

# Check if we're doing a hot reload build
if [ "$1" = "hot" ]; then
    print_status "Hot reload build - rebuilding shared library only..."
    
    build_shared_lib
    
    print_status "Hot reload build complete!"
    exit 0
fi

# Regular full build path continues here
mkdir -p $LIBS_DIR
mkdir -p $BUILD_DIR
mkdir -p $BUILD_DIR/assets

# Clone/download dependencies if not present
if [ ! -d "$RAYLIB_DIR" ]; then
    print_status "Cloning raylib..."
    git clone https://github.com/raysan5/raylib.git $RAYLIB_DIR
fi

if [ ! -d "$ENTT_DIR" ]; then
    print_status "Cloning ENTT..."
    git clone https://github.com/skypjack/entt.git $ENTT_DIR
fi

if [ ! -f "$LIBS_DIR/raygui.h" ]; then
    print_status "Downloading raygui..."
    curl -L https://raw.githubusercontent.com/raysan5/raygui/master/src/raygui.h -o $LIBS_DIR/raygui.h
fi

# Build raylib if not built
if [ ! -f "$RAYLIB_SRC/libraylib.a" ]; then
    print_status "Building raylib..."
    cd $RAYLIB_SRC
    make PLATFORM=PLATFORM_DESKTOP
    cd ../../..
fi

# Copy assets to build directory
print_status "Copying assets..."
cp -r assets/* $BUILD_DIR/assets/

# Build the game
build_shared_lib

print_status "Building main executable..."
g++ $SRC_DIR/main.cpp \
    $INCLUDES \
    $RAYLIB_LIBS \
    $SYSTEM_LIBS \
    $COMPILER_FLAGS \
    -o $BUILD_DIR/game

# Generate compilation database only if it doesn't exist
if [ ! -f "compile_commands.json" ]; then
    print_status "Generating compilation database..."
    bear -- g++ -shared -fPIC $SRC_DIR/game.cpp \
        $INCLUDES \
        $RAYLIB_LIBS \
        $SYSTEM_LIBS \
        $COMPILER_FLAGS \
        -o $BUILD_DIR/libgame.so

    bear -- g++ $SRC_DIR/main.cpp \
        $INCLUDES \
        $RAYLIB_LIBS \
        $SYSTEM_LIBS \
        $COMPILER_FLAGS \
        -o $BUILD_DIR/game
fi

print_status "Build complete!"

# Run the game from build directory
cd $BUILD_DIR
./game
