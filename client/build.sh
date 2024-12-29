#!/bin/bash

# Exit on error
set -e

RAYLIB_DIR="./includes/raylib"
RAYLIB_SRC="$RAYLIB_DIR/src"
INCLUDES_DIR="./includes"
SRC_DIR="./src"

# Create directories if they don't exist
mkdir -p $INCLUDES_DIR

# Clone raylib if not present
if [ ! -d "$RAYLIB_DIR" ]; then
    echo "Cloning raylib..."
    git clone https://github.com/raysan5/raylib.git $RAYLIB_DIR
    cd $RAYLIB_DIR
    cd ../..
fi

# Download latest raygui if not present
if [ ! -f "$INCLUDES_DIR/raygui.h" ]; then
    echo "Downloading raygui..."
    curl -L https://raw.githubusercontent.com/raysan5/raygui/refs/heads/master/src/raygui.h -o $INCLUDES_DIR/raygui.h
fi

# Build raylib if not built
if [ ! -f "$RAYLIB_SRC/libraylib.a" ]; then
    echo "Building raylib..."
    cd $RAYLIB_SRC
    make PLATFORM=PLATFORM_DESKTOP
    cd ../../..
fi

# Generate compilation database with bear only if it doesn't exist
if [ ! -f "compile_commands.json" ]; then
    echo "Generating compilation database..."
    bear -- g++ $SRC_DIR/main.cpp \
        -I $RAYLIB_SRC \
        -I $INCLUDES_DIR \
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
echo "Building main program..."
g++ $SRC_DIR/main.cpp \
    -I $RAYLIB_SRC \
    -I $INCLUDES_DIR \
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

echo "Build complete!"

./a.out
