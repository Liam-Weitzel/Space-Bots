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
    TEMP_NAME="libclient_${TIMESTAMP}.so"
    FINAL_NAME="libclient.so"
    
    # Build to a temporary file first
    g++ -shared -fPIC $CPP_FILES \
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
COMMON_LIBS_DIR="../libs"
RAYLIB_DIR="$LIBS_DIR/raylib"
RAYLIB_RLIGHTS="$RAYLIB_DIR/examples/shaders"
RAYLIB_REASINGS="$RAYLIB_DIR/examples/others"
RAYLIB_SRC="$RAYLIB_DIR/src"
RAYGUI_SRC="$LIBS_DIR/raygui/src"
STEAM_DIR="$COMMON_LIBS_DIR/steam/public/steam"
STEAM_LIB_DIR="$COMMON_LIBS_DIR/steam/redistributable_bin/linux64"
STEAM_LIB_FILE="$STEAM_LIB_DIR/libsteam_api.so"
RRES_SRC="$LIBS_DIR/rres/src"
SRC_DIR="./src"
GUIS_DIR="$SRC_DIR/guis"

CPP_FILES="$SRC_DIR/client.cpp $LIBS_DIR/utils_client.cpp $COMMON_LIBS_DIR/utils.cpp $COMMON_LIBS_DIR/utils_test.cpp $GUIS_DIR/main_menu.cpp $GUIS_DIR/settings.cpp"

# Common flags
INCLUDES="-I $COMMON_LIBS_DIR -I $LIBS_DIR -I $RAYLIB_SRC -I $RAYLIB_RLIGHTS -I $RAYLIB_REASINGS -I $RAYGUI_SRC -I $RRES_SRC -I $SRC_DIR -I $GUIS_DIR -I $STEAM_DIR"
RAYLIB_LIBS="-L $RAYLIB_SRC -L $RAYLIB_SRC/rtext -lraylib"
STEAM_LIBS=" -l steam_api -L $STEAM_LIB_DIR"
SYSTEM_LIBS="-lGL -lm -lpthread -ldl -lrt -lX11"
#Common flags
COMPILER_FLAGS="\
    -Wall \
    -Wextra \
    -pedantic \
    -Werror \
    -isystem $RAYLIB_SRC \
    -isystem $RAYGUI_SRC \
    -isystem $RRES_SRC \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-unused-but-set-variable \
    -Wno-unused-result \
    -Wno-format-security \
    -Wno-write-strings \
    -Wno-missing-field-initializers \
    -fno-gnu-unique \
    -Wl,-rpath,\$ORIGIN/"
#Debug flags
COMPILER_FLAGS+="\
    -Wno-error=cpp \
    -fsanitize=undefined \
    -fno-sanitize-recover=all \
    -O0 -g"
#Release flags
# COMPILER_FLAGS+="\
#     -O3 \
#     -flto \
#     -march=native \

# Build raylib if not built
if [ ! -f "$RAYLIB_SRC/libraylib.a" ]; then
    print_status "Building raylib..."
    cd $RAYLIB_SRC
    make PLATFORM=PLATFORM_DESKTOP
    cd ../../..
fi

# Copy steamworks library
cp $STEAM_LIB_FILE ./

print_status "Building prep models executable..."
g++ prep_models.cpp $CPP_FILES \
    $INCLUDES \
    $RAYLIB_LIBS \
    $SYSTEM_LIBS \
    $COMPILER_FLAGS \
    -o prep_models 

./prep_models

# Build the client
build_shared_lib

print_status "Building test executable..."
g++ $SRC_DIR/main_test.cpp $CPP_FILES \
    $INCLUDES \
    $RAYLIB_LIBS \
    $STEAM_LIBS \
    $SYSTEM_LIBS \
    $COMPILER_FLAGS \
    -o ./client_test

# Run the client tests
./client_test

# Check if we're doing a hot reload build
if [ "$1" = "hot" ]; then
    print_status "Hot reload build complete!"
    exit 0
fi

print_status "Building main executable..."
g++ $SRC_DIR/main.cpp $CPP_FILES \
    $INCLUDES \
    $RAYLIB_LIBS \
    $STEAM_LIBS \
    $SYSTEM_LIBS \
    $COMPILER_FLAGS \
    -o ./client

# Generate compilation database only if it doesn't exist
if [ ! -f "compile_commands.json" ]; then
    print_status "Generating compilation database..."
    bear -- g++ -shared -fPIC $CPP_FILES \
        $INCLUDES \
        $RAYLIB_LIBS \
        $STEAM_LIBS \
        $SYSTEM_LIBS \
        $COMPILER_FLAGS \
        -o ./libclient.so
fi

print_status "Build complete!"

# Run the client
./client
