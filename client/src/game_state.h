#pragma once
#include "raylib.h"
#include "raygui.h"
#include "gui.h"
#include "rlights.h"

struct GUI {
    GuiGuiState gui_state;
    int style;
};

struct RoverAssets {
    Model body;
    Vector3 bodyOffset = {0.0f, 0.5f, 0.0f};
    Vector3 bodyRotationAxis = {0.0f, 1.0f, 0.0f};
    Model wheel;
    Vector3 wheelOffsets[4] = {
        {0.0f, 0.5f, 0.0f},    // backLeft
        {-2.1f, 0.5f, 0.0f},   // backRight
        {0.0f, 0.5f, -1.25f},  // frontLeft
        {-2.1f, 0.5f, -1.25f}  // frontRight
    };
    Vector3 wheelRotationAxis = {0.0f, 1.0f, 0.0f};
    Model scan;
    Vector3 scanOffset = {0.0f, 0.5f, 0.0f};
    Vector3 scanRotationAxis = {0.0f, 1.0f, 0.0f};
};

struct ResourceManager {
    RoverAssets roverAssets;
};

struct TransientStorage {
    Shader shadowShader;
    RenderTexture2D shadowMap;
    Vector3 lightDir;
    int lightDirLoc;
    int lightVPLoc;
    int shadowMapLoc;
    ResourceManager resourceManager;
};

struct GameState {
    Camera3D camera;
    Camera3D lightCamera;
    rresCentralDir dir;
    GUI gui;
    int frameCount;
    float deltaTime;
    TransientStorage transientStorage;
};

//*********************
//  ECS COMPONENTS
//*********************

struct _Position {
    Vector3 position;
    Quaternion rotation;
};

struct _Rover {
    float wheelRotations[4] = {0.0f}; // [backLeft, backRight, frontLeft, frontRight]
    bool isScanning = false;
    Vector3 scale = {1.0f, 1.0f, 1.0f};
    Color tint = WHITE;
};
