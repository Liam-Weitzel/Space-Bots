#pragma once
#include "raylib.h"
#include "raygui.h"
#include "gui.h"
#include "rlights.h"
#include "utils.h"
#include <cstdint>

struct GUI {
    GuiGuiState gui_state;
    uint8_t style;
};

struct RoverAssets {
    Model* body;
    Vector3 bodyOffset = {0.0f, 0.5f, 0.0f};
    Vector3 bodyRotationAxis = {0.0f, 1.0f, 0.0f};
    Model* wheel;
    Vector3 wheelOffsets[4] = {
        {0.0f, 0.5f, 0.0f},    // backLeft
        {-2.1f, 0.5f, 0.0f},   // backRight
        {0.0f, 0.5f, -1.25f},  // frontLeft
        {-2.1f, 0.5f, -1.25f}  // frontRight
    };
    Vector3 wheelRotationAxis = {0.0f, 1.0f, 0.0f};
    Model* scan;
    Vector3 scanOffset = {0.0f, 0.5f, 0.0f};
    Vector3 scanRotationAxis = {0.0f, 1.0f, 0.0f};
    Vector3 scale = {1.0f, 1.0f, 1.0f};
    Color tint = WHITE;
};

struct ResourceManager {
    RoverAssets roverAssets;
};

struct Shaders {
    Shader shadowShader;
    RenderTexture2D shadowMap;
    Vector3 lightDir;
    int lightDirLoc;
    int lightVPLoc;
    int shadowMapLoc;
};

struct Cameras {
    Camera3D camera;
    Camera3D lightCamera;
};

struct GameState {
    uint32_t frameCount;
    float deltaTime;
    Arena& frameArena = *new Arena(KB(5));        // Clears every frame
    Arena& matchArena = *new Arena(MB(5));        // Clears every match
    Arena& reloadArena = *new Arena(MB(50));      // Clears on hot-reload
    Arena& permanentArena = *new Arena(MB(100));  // Doesn't clear on hot-reload
};

struct Rover {
    Vector3 position;
    Quaternion rotation;
    float wheelRotations[4] = {0.0f}; // [backLeft, backRight, frontLeft, frontRight]
    bool isScanning = false;
}; // Stored in generational sparse set in match arena
