#pragma once
#include "entt.hpp"
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
    Vector3 bodyTranlation = {0.0f, 0.5f, 0.0f};
    Model wheel;
    Vector3 blWheelTranslation = {0.0f, 0.5f, 0.0f};
    Vector3 brWheelTranslation = {-2.1f, 0.5f, 0.0f};
    Vector3 flWheelTranslation = {0.0f, 0.5f, -1.25f};
    Vector3 frWheelTranslation = {-2.1f, 0.5f, -1.25f};
    Model scan;
    Vector3 scanTranslation = {0.0f, 0.5f, 0.0f};
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
    entt::registry registry;
    GUI gui;
    int frameCount;
    float deltaTime;
    TransientStorage transientStorage;
};

//*********************
//  ECS COMPONENTS
//*********************

struct _Transform {
    Vector3 translation;
    Quaternion rotation;
};

struct _Position {
    Vector3 position;
    Quaternion rotation;
};

struct _Rover {
    float leftBackWheelRotation;
    float rightBackWheelRotation;
    float leftFrontWheelRotation;
    float rightFrontWheelRotation;
};
