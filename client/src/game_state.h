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

struct TransientStorage {
    Shader shadowShader;
    RenderTexture2D shadowMap;
    Vector3 lightDir;
    int lightDirLoc;
    int lightVPLoc;
    int shadowMapLoc;
    Mesh roverMesh;
    Model roverModel;
    Model cubeModel;
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
