#pragma once
#include "raylib.h"
#include "raygui.h"
#include "gui.h"
#include "rlights.h"
#include "rlgl.h"
#include "utils.h"
#include <cstdint>

struct GUI { // Permanent
    GuiGuiState gui_state;
    uint8_t style;
};

struct RoverAssets { // Reload
    Model* body = nullptr;
    Vector3 bodyOffset = {0.0f, 0.5f, 0.0f};
    Vector3 bodyRotationAxis = {0.0f, 1.0f, 0.0f};
    Model* wheel = nullptr;
    Vector3 wheelOffsets[4] = {
        {0.0f, 0.5f, 0.0f},    // backLeft
        {-2.1f, 0.5f, 0.0f},   // backRight
        {0.0f, 0.5f, -1.25f},  // frontLeft
        {-2.1f, 0.5f, -1.25f}  // frontRight
    };
    Vector3 wheelRotationAxis = {0.0f, 1.0f, 0.0f};
    Model* scan = nullptr;
    Vector3 scanOffset = {0.0f, 0.5f, 0.0f};
    Vector3 scanRotationAxis = {0.0f, 1.0f, 0.0f};
    Color tint = WHITE;

    void cleanup() {
        if (body) {
            UnloadModel(*body);
            body = nullptr;
        }
        if (wheel) {
            UnloadModel(*wheel);
            wheel = nullptr;
        }
        if (scan) {
            UnloadModel(*scan);
            scan = nullptr;
        }
    }
};

struct ResourceManager { // Reload
    rresCentralDir dir;
    RoverAssets roverAssets;

    void cleanup() {
        rresUnloadCentralDirectory(dir);
        roverAssets.cleanup();
    }
};

struct Shaders { // Reload
    Shader shadowShader;
    RenderTexture2D shadowMap;
    Vector3 lightDir;
    int lightDirLoc;
    int lightVPLoc;
    int shadowMapLoc;

    void cleanup() {
        UnloadShader(shadowShader);
        UnloadRenderTexture(shadowMap);

        if (shadowMap.id > 0)
            rlUnloadFramebuffer(shadowMap.id);
    }
};

struct Cameras { // Permanent
    Camera3D camera;
    Camera3D lightCamera;
};

struct RenderResources {
    Cameras* cameras;                  // Permanent
    Shaders* shaders;                  // Reload
    ResourceManager* resourceManager;  // Reload
    rresCentralDir* dir;               // Reload
    GUI* gui;                          // Permanent

    void cleanup() {
        shaders->cleanup();
        resourceManager->cleanup();
        rresUnloadCentralDirectory(*dir);
    }
};

struct GameState {
    uint32_t frameCount;
    float deltaTime;

    // Direct pointers to static arena-managed resources
    RenderResources renderResources;

    // Arenas
    Arena frameArena;        // Clears every frame
    Arena matchArena;        // Clears every match
    Arena reloadArena;       // Clears on hot-reload
    Arena permanentArena;    // Doesn't clear on hot-reload

    GameState()
        : frameArena(KB(5))
        , matchArena(MB(5))
        , reloadArena(MB(50))
        , permanentArena(MB(100))
    {
        frameArena.create_map_ct<const char*, void*, 100>();
        matchArena.create_map_ct<const char*, void*, 100>();
        reloadArena.create_map_ct<const char*, void*, 100>();
        permanentArena.create_map_ct<const char*, void*, 100>();
    }
};

struct Rover { // Stored in generational sparse set in match arena
    Transform transform;
    ArrayCT<float, 4> wheelRotations; // [backLeft, backRight, frontLeft, frontRight]
    bool isScanning = false;
};
