#!/bin/bash

# Exit on error
set -e

# Function for red echo messages
print_status() {
    echo -e "\e[31m$1\e[0m"
}

LIBS_DIR="./libs"
RAYLIB_DIR="$LIBS_DIR/raylib"
RAYLIB_SRC="$RAYLIB_DIR/src"
ENTT_DIR="$LIBS_DIR/entt"
SRC_DIR="./src"

# Create directories if they don't exist
mkdir -p $LIBS_DIR

# Clone raylib if not present
if [ ! -d "$RAYLIB_DIR" ]; then
    print_status "Cloning raylib..."
    git clone https://github.com/raysan5/raylib.git $RAYLIB_DIR
    cd $RAYLIB_DIR
    cd ../..
fi

# Clone ENTT if not present
if [ ! -d "$ENTT_DIR" ]; then
    print_status "Cloning ENTT..."
    git clone https://github.com/skypjack/entt.git $ENTT_DIR
fi

# Download latest raygui if not present
if [ ! -f "$LIBS_DIR/raygui.h" ]; then
    print_status "Downloading raygui..."
    curl -L https://raw.githubusercontent.com/raysan5/raygui/refs/heads/master/src/raygui.h -o $LIBS_DIR/raygui.h
fi

# Build raylib if not built
if [ ! -f "$RAYLIB_SRC/libraylib.a" ]; then
    print_status "Building raylib..."
    cd $RAYLIB_SRC
    make PLATFORM=PLATFORM_DESKTOP
    cd ../../..
fi

# Generate compilation database with bear only if it doesn't exist
if [ ! -f "compile_commands.json" ]; then
    print_status "Generating compilation database..."
    bear -- g++ $SRC_DIR/main.cpp \
        -I $RAYLIB_SRC \
        -I $LIBS_DIR \
        -I $ENTT_DIR/src/entt \
        -I $SRC_DIR \
        -L $RAYLIB_SRC \
        -lraylib \
        -lGL \
        -lm \
        -lpthread \
        -ldl \
        -lrt \
        -lX11
fi

# Compile the main program
print_status "Building game..."
g++ $SRC_DIR/main.cpp \
    -I $RAYLIB_SRC \
    -I $LIBS_DIR \
    -I $ENTT_DIR/src/entt \
    -I $SRC_DIR \
    -L $RAYLIB_SRC \
    -L $RAYLIB_SRC/rtext \
    -lraylib \
    -lGL \
    -lm \
    -lpthread \
    -ldl \
    -lrt \
    -lX11

print_status "Build complete!"

./a.out
