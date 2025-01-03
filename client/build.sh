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
        $STEAM_LIBS \
        $SYSTEM_LIBS \
        $COMPILER_FLAGS \
        -o ./$TEMP_NAME

    # Move temp file to final name
    mv ./$TEMP_NAME ./$FINAL_NAME
    sync
}

LIBS_DIR="./libs"
RAYLIB_DIR="$LIBS_DIR/raylib"
RAYLIB_SRC="$RAYLIB_DIR/src"
ENTT_DIR="$LIBS_DIR/entt"
STEAM_DIR="$LIBS_DIR/steam"
STEAM_LIB_DIR="$STEAM_DIR/linux64"
SRC_DIR="./src"

# Common flags
INCLUDES="-I $RAYLIB_SRC -I $LIBS_DIR -I $ENTT_DIR/src/entt -I $SRC_DIR -I $STEAM_DIR"
RAYLIB_LIBS="-L $RAYLIB_SRC -L $RAYLIB_SRC/rtext -lraylib"
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

# Clone/download dependencies if not present
if [ ! -d "$RAYLIB_DIR" ]; then
    print_status "Cloning raylib..."
    git clone https://github.com/raysan5/raylib.git $RAYLIB_DIR
    cd $RAYLIB_DIR
    git reset --hard 0f6e85a975f637e14b1fed3ff6022a0e0008e620
    cd ..
fi

if [ ! -d "$ENTT_DIR" ]; then
    print_status "Cloning ENTT..."
    git clone https://github.com/skypjack/entt.git $ENTT_DIR
    cd $ENTT_DIR
    git reset --hard 9a19f9aa2f5c7d822ceeea06bce1b0a882c2cb97
    cd ..
fi

if [ ! -f "$LIBS_DIR/raygui.h" ]; then
    print_status "Downloading raygui..."
    curl -L https://raw.githubusercontent.com/raysan5/raygui/1f813711063f0ff01bd3a89d50ebc17c5412e83c/src/raygui.h -o $LIBS_DIR/raygui.h
fi

# Build raylib if not built
if [ ! -f "$RAYLIB_SRC/libraylib.a" ]; then
    print_status "Building raylib..."
    cd $RAYLIB_SRC
    make PLATFORM=PLATFORM_DESKTOP
    cd ../../..
fi

# Copy steamworks library
cp $STEAM_LIB_DIR/libsteam_api.so ./

# Build the game
build_shared_lib

print_status "Building main executable..."
g++ $SRC_DIR/main.cpp \
    $INCLUDES \
    $RAYLIB_LIBS \
    $STEAM_LIBS \
    $SYSTEM_LIBS \
    $COMPILER_FLAGS \
    -o ./game

# Generate compilation database only if it doesn't exist
if [ ! -f "compile_commands.json" ]; then
    print_status "Generating compilation database..."
    bear -- g++ -shared -fPIC $SRC_DIR/game.cpp \
        $INCLUDES \
        $RAYLIB_LIBS \
        $STEAM_LIBS \
        $SYSTEM_LIBS \
        $COMPILER_FLAGS \
        -o ./libgame.so

    bear -- g++ $SRC_DIR/main.cpp \
        $INCLUDES \
        $RAYLIB_LIBS \
        $STEAM_LIBS \
        $SYSTEM_LIBS \
        $COMPILER_FLAGS \
        -o ./game
fi

print_status "Build complete!"

# Run the game
./game
