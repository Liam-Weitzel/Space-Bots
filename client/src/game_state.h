#pragma once
#include "utils.h"
#include "utils_client.h"

struct GUI { // Permanent
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
};

struct ResourceManager { // Reload
  rresCentralDir dir;
  RoverAssets roverAssets;

  void reload() {
    // Don't unload models as they are freed when arena is cleared
    rresUnloadCentralDirectory(dir);
  }
};

struct Shaders { // Reload
  Shader shadowShader;
  RenderTexture2D shadowMap;
  Vector3 lightDir;
  int lightDirLoc;
  int lightVPLoc;
  int shadowMapLoc;

  void reload() {
    UnloadShader(shadowShader);
    UnloadRenderTexture(shadowMap);

    if (shadowMap.id > 0) rlUnloadFramebuffer(shadowMap.id);
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

  void reload() {
    shaders->reload();
    resourceManager->reload();
    shaders = nullptr;
    resourceManager = nullptr;
    dir = nullptr;
  }
};

enum class GameMode {
  MENU,
  REALTIME,
  AUTOBATTLE
};

struct GameState {
  uint32_t frameCount = 0;
  float deltaTime = 0;
  GameMode gameMode = GameMode::MENU;

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
    frameArena.create_map_ct<const char*, void*, ArenaIndexSize>();
    matchArena.create_map_ct<const char*, void*, ArenaIndexSize>();
    reloadArena.create_map_ct<const char*, void*, ArenaIndexSize>();
    permanentArena.create_map_ct<const char*, void*, ArenaIndexSize>();
  }
};

struct Rover { // Stored in generational sparse set in match arena
  Transform transform;
  ArrayCT<float, 4> wheelRotations; // [backLeft, backRight, frontLeft, frontRight]
  bool isScanning = false;
};
