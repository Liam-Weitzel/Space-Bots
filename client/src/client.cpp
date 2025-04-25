#include "game_state.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "utils.h"
#include "utils_client.h"
#include <cstddef>
#include <cstdint>

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "raygui.h"

#define GUI_GUI_IMPLEMENTATION
#include "gui.h"

#define RRES_IMPLEMENTATION
#include "rres.h"

#define RRES_RAYLIB_IMPLEMENTATION
#include "rres-raylib.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#define GLSL_VERSION 330
#define SHADOWMAP_RESOLUTION 2048

void init(GameState& state) {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(800, 450, "video game");
  SetTargetFPS(1000);

  bool isReload = state.permanentArena.size() > 1700; // is our current load a hot code reload?

  rresCentralDir& dir = state.reloadArena.create<rresCentralDir>();
  dir = rresLoadCentralDirectory("resources.rres");
  state.renderResources.dir = &dir;

  ResourceManager& resourceManager = state.reloadArena.create<ResourceManager>();
  state.renderResources.resourceManager = &resourceManager;

  int idRoverBody = rresGetResourceId(dir, "rover_body.bin");
  rresResourceChunk chunkRoverBody = rresLoadResourceChunk("resources.rres", idRoverBody);
  Model& body = LoadModelFromChunk(chunkRoverBody, state.reloadArena);
  resourceManager.roverAssets.body = &body;

  int idRoverScan = rresGetResourceId(dir, "rover_scan.bin");
  rresResourceChunk chunkRoverScan = rresLoadResourceChunk("resources.rres", idRoverScan);
  Model& scan = LoadModelFromChunk(chunkRoverScan, state.reloadArena);
  resourceManager.roverAssets.scan = &scan;

  int idRoverWheel = rresGetResourceId(dir, "rover_wheel.bin");
  rresResourceChunk chunkRoverWheel = rresLoadResourceChunk("resources.rres", idRoverWheel);
  Model& wheel = LoadModelFromChunk(chunkRoverWheel, state.reloadArena);
  resourceManager.roverAssets.wheel = &wheel;

  Shaders& shaders = state.reloadArena.create<Shaders>();
  state.renderResources.shaders = &shaders;

  int shadowVsId = rresGetResourceId(dir, "shadowmap.vs");
  int shadowFsId = rresGetResourceId(dir, "shadowmap.fs");
  rresResourceChunk shadowVsChunk = rresLoadResourceChunk("resources.rres", shadowVsId);
  rresResourceChunk shadowFsChunk = rresLoadResourceChunk("resources.rres", shadowFsId);
  char* vsCode = cleanShaderCode(shadowVsChunk);
  char* fsCode = cleanShaderCode(shadowFsChunk);
  shaders.shadowShader = LoadShaderFromMemory(vsCode, fsCode);
  delete[] vsCode;
  delete[] fsCode;

  rresUnloadResourceChunk(chunkRoverBody);
  rresUnloadResourceChunk(chunkRoverScan);
  rresUnloadResourceChunk(chunkRoverWheel);
  rresUnloadResourceChunk(shadowVsChunk);
  rresUnloadResourceChunk(shadowFsChunk);

  shaders.shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shaders.shadowShader, "viewPos");
  shaders.lightDir = Vector3Normalize(Vector3{0.35f, -1.0f, -0.35f});
  Color lightColor = WHITE;
  Vector4 lightColorNormalized = ColorNormalize(lightColor);
  shaders.lightDirLoc = GetShaderLocation(shaders.shadowShader, "lightDir");
  int lightColLoc = GetShaderLocation(shaders.shadowShader, "lightColor");
  SetShaderValue(shaders.shadowShader,
                 shaders.lightDirLoc,
                 &shaders.lightDir, SHADER_UNIFORM_VEC3);
  SetShaderValue(shaders.shadowShader, lightColLoc,
                 &lightColorNormalized, SHADER_UNIFORM_VEC4);
  int ambientLoc =
      GetShaderLocation(shaders.shadowShader, "ambient");
  float ambient[4] = {0.1f, 0.1f, 0.1f, 1.0f};
  SetShaderValue(shaders.shadowShader, ambientLoc, ambient,
                 SHADER_UNIFORM_VEC4);
  shaders.lightVPLoc =
      GetShaderLocation(shaders.shadowShader, "lightVP");
  shaders.shadowMapLoc =
      GetShaderLocation(shaders.shadowShader, "shadowMap");
  int shadowMapResolution = SHADOWMAP_RESOLUTION;
  SetShaderValue(shaders.shadowShader,
                 GetShaderLocation(shaders.shadowShader,
                                   "shadowMapResolution"),
                 &shadowMapResolution, SHADER_UNIFORM_INT);

  for (int i = 0; i < resourceManager.roverAssets.body->materialCount; i++) {
    resourceManager.roverAssets.body->materials[i].shader =
        shaders.shadowShader;
  }

  for (int i = 0; i < resourceManager.roverAssets.wheel->materialCount; i++) {
    resourceManager.roverAssets.wheel->materials[i].shader =
        shaders.shadowShader;
  }

  shaders.shadowMap.id = rlLoadFramebuffer(); // Load an empty framebuffer
  shaders.shadowMap.texture.width = SHADOWMAP_RESOLUTION;
  shaders.shadowMap.texture.height = SHADOWMAP_RESOLUTION;

  if (shaders.shadowMap.id > 0) {
    rlEnableFramebuffer(shaders.shadowMap.id);

    // Create depth texture
    // We don't need a color texture for the shadowmap
    shaders.shadowMap.depth.id = rlLoadTextureDepth(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION, false);
    shaders.shadowMap.depth.width = SHADOWMAP_RESOLUTION;
    shaders.shadowMap.depth.height = SHADOWMAP_RESOLUTION;
    shaders.shadowMap.depth.format = 19; // DEPTH_COMPONENT_24BIT?
    shaders.shadowMap.depth.mipmaps = 1;

    // Attach depth texture to FBO
    rlFramebufferAttach(shaders.shadowMap.id,
                        shaders.shadowMap.depth.id,
                        RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

    // Check if fbo is complete with attachments (valid)
    if (rlFramebufferComplete(shaders.shadowMap.id))
      TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully",
               shaders.shadowMap.id);

    rlDisableFramebuffer();
  } else
    TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

  int idIcons = rresGetResourceId(dir, "icons.rgi");
  rresResourceChunk chunkIcons =
      rresLoadResourceChunk("resources.rres", idIcons);
  GuiLoadIconsFromMemory((const unsigned char *)chunkIcons.data.raw,
                         chunkIcons.info.baseSize, "icons");
  rresUnloadResourceChunk(chunkIcons);


  if(!isReload) {
    Cameras& cameras = state.permanentArena.create<Cameras>();
    state.renderResources.cameras = &cameras;

    cameras.camera.position = Vector3{10.0f, 10.0f, 10.0f};
    cameras.camera.target = Vector3Zero();
    cameras.camera.projection = CAMERA_PERSPECTIVE;
    cameras.camera.up = Vector3{0.0f, 1.0f, 0.0f};
    cameras.camera.fovy = 45.0f;

    // For the shadowmapping algorithm, we will be rendering everything from the
    // light's point of view
    cameras.lightCamera.position =
        Vector3Scale(shaders.lightDir, -15.0f);
    cameras.lightCamera.target = Vector3Zero();
    // Use an orthographic projection for directional lights
    cameras.lightCamera.projection = CAMERA_ORTHOGRAPHIC;
    cameras.lightCamera.up = Vector3{0.0f, 1.0f, 0.0f};
    cameras.lightCamera.fovy = 20.0f;

    GuiLoadStyleDefault();
    GUI& gui = state.permanentArena.create<GUI>();
    state.renderResources.gui = &gui;
    GuiGuiState gui_state = InitGuiGui();
    gui.gui_state = gui_state;
  }
}

void render(GameState& state) {
  BeginDrawing();

  Matrix lightView;
  Matrix lightProj;

  Shaders& shaders = *state.renderResources.shaders;
  BeginTextureMode(shaders.shadowMap);
  ClearBackground(WHITE);
  Cameras& cameras = *state.renderResources.cameras;
  BeginMode3D(cameras.lightCamera);
    lightView = rlGetMatrixModelview();
    lightProj = rlGetMatrixProjection();
    ResourceManager& resourceManager = *state.renderResources.resourceManager;

    RoverAssets& roverAssets = resourceManager.roverAssets;
    DrawModelEx(*roverAssets.body, roverAssets.bodyOffset, roverAssets.bodyRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, roverAssets.tint);
    DrawModelEx(*roverAssets.wheel, roverAssets.wheelOffsets[0], roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, roverAssets.tint);
    DrawModelEx(*roverAssets.wheel, roverAssets.wheelOffsets[1], roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, roverAssets.tint);
    DrawModelEx(*roverAssets.wheel, roverAssets.wheelOffsets[2], roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, roverAssets.tint);
    DrawModelEx(*roverAssets.wheel, roverAssets.wheelOffsets[3], roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, roverAssets.tint);

  EndMode3D();
  EndTextureMode();

  Matrix lightViewProj = MatrixMultiply(lightView, lightProj);
  ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
  SetShaderValueMatrix(shaders.shadowShader,
                       shaders.lightVPLoc, lightViewProj);

  rlEnableShader(shaders.shadowShader.id);
  int slot = 10; // Can be anything 0 to 15, but 0 will probably be taken up
  rlActiveTextureSlot(10);
  rlEnableTexture(shaders.shadowMap.depth.id);
  rlSetUniform(shaders.shadowMapLoc, &slot, SHADER_UNIFORM_INT, 1);

  BeginMode3D(cameras.camera);
  DrawModelEx(*roverAssets.scan, roverAssets.scanOffset, roverAssets.scanRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, roverAssets.tint);
  DrawModelEx(*roverAssets.body, roverAssets.bodyOffset, roverAssets.bodyRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, roverAssets.tint);
  DrawModelEx(*roverAssets.wheel, roverAssets.wheelOffsets[0], roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, roverAssets.tint);
  DrawModelEx(*roverAssets.wheel, roverAssets.wheelOffsets[1], roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, roverAssets.tint);
  DrawModelEx(*roverAssets.wheel, roverAssets.wheelOffsets[2], roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, roverAssets.tint);
  DrawModelEx(*roverAssets.wheel, roverAssets.wheelOffsets[3], roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, roverAssets.tint);
  EndMode3D();

  DrawFPS(10, 10);

  DrawText("Use the arrow keys to move the light", 10, 40, 20,
           GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)));

  GUI& gui = *state.renderResources.gui;
  rresCentralDir& dir = *state.renderResources.dir;
  GuiGui(&gui, &dir);
  EndDrawing();
}

void update(GameState& state) {
  Cameras& cameras = *state.renderResources.cameras;
  Shaders& shaders = *state.renderResources.shaders;
  SetShaderValue(
      shaders.shadowShader,
      shaders.shadowShader.locs[SHADER_LOC_VECTOR_VIEW],
      &cameras.camera.position, SHADER_UNIFORM_VEC3);
  UpdateCamera(&cameras.camera, CAMERA_ORBITAL);

  const float cameraSpeed = 0.05f;
  if (IsKeyDown(KEY_LEFT)) {
    if (shaders.lightDir.x < 0.6f)
      shaders.lightDir.x +=
          cameraSpeed * 60.0f * state.deltaTime;
  }
  if (IsKeyDown(KEY_RIGHT)) {
    if (shaders.lightDir.x > -0.6f)
      shaders.lightDir.x -=
          cameraSpeed * 60.0f * state.deltaTime;
  }
  if (IsKeyDown(KEY_UP)) {
    if (shaders.lightDir.z < 0.6f)
      shaders.lightDir.z +=
          cameraSpeed * 60.0f * state.deltaTime;
  }
  if (IsKeyDown(KEY_DOWN)) {
    if (shaders.lightDir.z > -0.6f)
      shaders.lightDir.z -=
          cameraSpeed * 60.0f * state.deltaTime;
  }
  shaders.lightDir =
      Vector3Normalize(shaders.lightDir);
  cameras.lightCamera.position =
      Vector3Scale(shaders.lightDir, -15.0f);
  SetShaderValue(shaders.shadowShader,
                 shaders.lightDirLoc,
                 &shaders.lightDir, SHADER_UNIFORM_VEC3);
}

void reload(GameState& state) {
  state.renderResources.reload();
  state.reloadArena.clear();
  CloseWindow();
}

EXPORT_FN void client_main(GameState& state) {
  init(state);
  uint64_t last_write_time = get_timestamp("./libclient.so");
  while (!WindowShouldClose()) {
    if (last_write_time != get_timestamp("./libclient.so"))
      break;
    state.frameCount++;
    state.deltaTime = GetFrameTime();
    update(state);
    render(state);
    state.frameArena.clear();
  }
  reload(state);
}
