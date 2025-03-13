#include "game_state.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "utils_client.h"

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
#define SHADOWMAP_RESOLUTION 1024

void render(GameState *state) {
  BeginDrawing();

  Matrix lightView;
  Matrix lightProj;
  BeginTextureMode(state->transientStorage.shadowMap);
  ClearBackground(WHITE);
  BeginMode3D(state->lightCamera);
    lightView = rlGetMatrixModelview();
    lightProj = rlGetMatrixProjection();

    DrawModelEx(state->transientStorage.resourceManager.roverAssets.body, state->transientStorage.resourceManager.roverAssets.bodyOffset, state->transientStorage.resourceManager.roverAssets.bodyRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, WHITE);
    DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.wheelOffsets[0], state->transientStorage.resourceManager.roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, WHITE);
    DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.wheelOffsets[1], state->transientStorage.resourceManager.roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, WHITE);
    DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.wheelOffsets[2], state->transientStorage.resourceManager.roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, WHITE);
    DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.wheelOffsets[3], state->transientStorage.resourceManager.roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, WHITE);
  EndMode3D();
  EndTextureMode();

  Matrix lightViewProj = MatrixMultiply(lightView, lightProj);
  ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
  SetShaderValueMatrix(state->transientStorage.shadowShader,
                       state->transientStorage.lightVPLoc, lightViewProj);

  rlEnableShader(state->transientStorage.shadowShader.id);
  int slot = 10; // Can be anything 0 to 15, but 0 will probably be taken up
  rlActiveTextureSlot(10);
  rlEnableTexture(state->transientStorage.shadowMap.depth.id);
  rlSetUniform(state->transientStorage.shadowMapLoc, &slot, SHADER_UNIFORM_INT, 1);

  BeginMode3D(state->camera);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.scan, state->transientStorage.resourceManager.roverAssets.scanOffset, state->transientStorage.resourceManager.roverAssets.scanRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.body, state->transientStorage.resourceManager.roverAssets.bodyOffset, state->transientStorage.resourceManager.roverAssets.bodyRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.wheelOffsets[0], state->transientStorage.resourceManager.roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.wheelOffsets[1], state->transientStorage.resourceManager.roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.wheelOffsets[2], state->transientStorage.resourceManager.roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.wheelOffsets[3], state->transientStorage.resourceManager.roverAssets.wheelRotationAxis, 0.0f, {1.0f, 1.0f, 1.0f}, WHITE);
  EndMode3D();

  DrawFPS(10, 10);

  DrawText("Use key [Y] to toggle lights", 10, 40, 20,
           GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)));

  GuiGui(&state->gui, &state->dir);
  EndDrawing();
}

void cleanup(GameState *state) {
  UnloadShader(state->transientStorage.shadowShader);
  UnloadModel(state->transientStorage.resourceManager.roverAssets.body);
  UnloadModel(state->transientStorage.resourceManager.roverAssets.scan);
  UnloadModel(state->transientStorage.resourceManager.roverAssets.wheel);
  if (state->transientStorage.shadowMap.id > 0)
    rlUnloadFramebuffer(state->transientStorage.shadowMap.id);
  rresUnloadCentralDirectory(state->dir);
  CloseWindow();
}

void init(GameState *state) {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(800, 450, "video game");
  SetTargetFPS(120);

  Arena* arena = new Arena(KB(32));
  state->dir = rresLoadCentralDirectory("resources.rres");

  int idRoverBody = rresGetResourceId(state->dir, "rover_body.bin");
  rresResourceChunk chunkRoverBody = rresLoadResourceChunk("resources.rres", idRoverBody);
  state->transientStorage.resourceManager.roverAssets.body = *LoadModelFromChunk(chunkRoverBody, arena);

  int idRoverScan = rresGetResourceId(state->dir, "rover_scan.bin");
  rresResourceChunk chunkRoverScan = rresLoadResourceChunk("resources.rres", idRoverScan);
  state->transientStorage.resourceManager.roverAssets.scan = *LoadModelFromChunk(chunkRoverScan, arena);

  int idRoverWheel = rresGetResourceId(state->dir, "rover_wheel.bin");
  rresResourceChunk chunkRoverWheel = rresLoadResourceChunk("resources.rres", idRoverWheel);
  state->transientStorage.resourceManager.roverAssets.wheel = *LoadModelFromChunk(chunkRoverWheel, arena);

  int shadowVsId = rresGetResourceId(state->dir, "shadowmap.vs");
  int shadowFsId = rresGetResourceId(state->dir, "shadowmap.fs");
  rresResourceChunk shadowVsChunk =
      rresLoadResourceChunk("resources.rres", shadowVsId);
  rresResourceChunk shadowFsChunk =
      rresLoadResourceChunk("resources.rres", shadowFsId);
  state->transientStorage.shadowShader = LoadShaderFromMemory(
      (const char *)shadowVsChunk.data.raw, // vertex shader code
      (const char *)shadowFsChunk.data.raw  // fragment shader code
  );

  rresUnloadResourceChunk(chunkRoverBody);
  rresUnloadResourceChunk(chunkRoverScan);
  rresUnloadResourceChunk(chunkRoverWheel); //shader compilation breaks if this stuff is unloaded too early...
  rresUnloadResourceChunk(shadowVsChunk);
  rresUnloadResourceChunk(shadowFsChunk);

  int idIcons = rresGetResourceId(state->dir, "icons.rgi");
  rresResourceChunk chunkIcons =
      rresLoadResourceChunk("resources.rres", idIcons);
  GuiLoadIconsFromMemory((const unsigned char *)chunkIcons.data.raw,
                         chunkIcons.info.baseSize, "icons");
  rresUnloadResourceChunk(chunkIcons);

  GuiLoadStyleDefault();

  state->camera.position = (Vector3){10.0f, 10.0f, 10.0f};
  state->camera.target = Vector3Zero();
  state->camera.projection = CAMERA_PERSPECTIVE;
  state->camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  state->camera.fovy = 45.0f;

  state->transientStorage.shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(state->transientStorage.shadowShader, "viewPos");
  state->transientStorage.lightDir = Vector3Normalize((Vector3){0.35f, -1.0f, -0.35f});
  Color lightColor = WHITE;
  Vector4 lightColorNormalized = ColorNormalize(lightColor);
  state->transientStorage.lightDirLoc = GetShaderLocation(state->transientStorage.shadowShader, "lightDir");
  int lightColLoc = GetShaderLocation(state->transientStorage.shadowShader, "lightColor");
  SetShaderValue(state->transientStorage.shadowShader,
                 state->transientStorage.lightDirLoc,
                 &state->transientStorage.lightDir, SHADER_UNIFORM_VEC3);
  SetShaderValue(state->transientStorage.shadowShader, lightColLoc,
                 &lightColorNormalized, SHADER_UNIFORM_VEC4);
  int ambientLoc =
      GetShaderLocation(state->transientStorage.shadowShader, "ambient");
  float ambient[4] = {0.1f, 0.1f, 0.1f, 1.0f};
  SetShaderValue(state->transientStorage.shadowShader, ambientLoc, ambient,
                 SHADER_UNIFORM_VEC4);
  state->transientStorage.lightVPLoc =
      GetShaderLocation(state->transientStorage.shadowShader, "lightVP");
  state->transientStorage.shadowMapLoc =
      GetShaderLocation(state->transientStorage.shadowShader, "shadowMap");
  int shadowMapResolution = SHADOWMAP_RESOLUTION;
  SetShaderValue(state->transientStorage.shadowShader,
                 GetShaderLocation(state->transientStorage.shadowShader,
                                   "shadowMapResolution"),
                 &shadowMapResolution, SHADER_UNIFORM_INT);

  for (int i = 0; i < state->transientStorage.resourceManager.roverAssets.body.materialCount; i++) {
    state->transientStorage.resourceManager.roverAssets.body.materials[i].shader =
        state->transientStorage.shadowShader;
  }

  for (int i = 0; i < state->transientStorage.resourceManager.roverAssets.wheel.materialCount; i++) {
    state->transientStorage.resourceManager.roverAssets.wheel.materials[i].shader =
        state->transientStorage.shadowShader;
  }

  state->transientStorage.shadowMap.id =
      rlLoadFramebuffer(); // Load an empty framebuffer
  state->transientStorage.shadowMap.texture.width = SHADOWMAP_RESOLUTION;
  state->transientStorage.shadowMap.texture.height = SHADOWMAP_RESOLUTION;

  if (state->transientStorage.shadowMap.id > 0) {
    rlEnableFramebuffer(state->transientStorage.shadowMap.id);

    // Create depth texture
    // We don't need a color texture for the shadowmap
    state->transientStorage.shadowMap.depth.id =
        rlLoadTextureDepth(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION, false);
    state->transientStorage.shadowMap.depth.width = SHADOWMAP_RESOLUTION;
    state->transientStorage.shadowMap.depth.height = SHADOWMAP_RESOLUTION;
    state->transientStorage.shadowMap.depth.format =
        19; // DEPTH_COMPONENT_24BIT?
    state->transientStorage.shadowMap.depth.mipmaps = 1;

    // Attach depth texture to FBO
    rlFramebufferAttach(state->transientStorage.shadowMap.id,
                        state->transientStorage.shadowMap.depth.id,
                        RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

    // Check if fbo is complete with attachments (valid)
    if (rlFramebufferComplete(state->transientStorage.shadowMap.id))
      TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully",
               state->transientStorage.shadowMap.id);

    rlDisableFramebuffer();
  } else
    TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

  // For the shadowmapping algorithm, we will be rendering everything from the
  // light's point of view
  state->lightCamera.position =
      Vector3Scale(state->transientStorage.lightDir, -15.0f);
  state->lightCamera.target = Vector3Zero();
  // Use an orthographic projection for directional lights
  state->lightCamera.projection = CAMERA_ORTHOGRAPHIC;
  state->lightCamera.up = (Vector3){0.0f, 1.0f, 0.0f};
  state->lightCamera.fovy = 20.0f;

  GuiGuiState gui_state = InitGuiGui();
  state->gui.gui_state = gui_state;
}

void update(GameState *state) {
  Vector3 cameraPos = state->camera.position;
  SetShaderValue(
      state->transientStorage.shadowShader,
      state->transientStorage.shadowShader.locs[SHADER_LOC_VECTOR_VIEW],
      &cameraPos, SHADER_UNIFORM_VEC3);
  UpdateCamera(&state->camera, CAMERA_ORBITAL);

  const float cameraSpeed = 0.05f;
  if (IsKeyDown(KEY_LEFT)) {
    if (state->transientStorage.lightDir.x < 0.6f)
      state->transientStorage.lightDir.x +=
          cameraSpeed * 60.0f * state->deltaTime;
  }
  if (IsKeyDown(KEY_RIGHT)) {
    if (state->transientStorage.lightDir.x > -0.6f)
      state->transientStorage.lightDir.x -=
          cameraSpeed * 60.0f * state->deltaTime;
  }
  if (IsKeyDown(KEY_UP)) {
    if (state->transientStorage.lightDir.z < 0.6f)
      state->transientStorage.lightDir.z +=
          cameraSpeed * 60.0f * state->deltaTime;
  }
  if (IsKeyDown(KEY_DOWN)) {
    if (state->transientStorage.lightDir.z > -0.6f)
      state->transientStorage.lightDir.z -=
          cameraSpeed * 60.0f * state->deltaTime;
  }
  state->transientStorage.lightDir =
      Vector3Normalize(state->transientStorage.lightDir);
  state->lightCamera.position =
      Vector3Scale(state->transientStorage.lightDir, -15.0f);
  SetShaderValue(state->transientStorage.shadowShader,
                 state->transientStorage.lightDirLoc,
                 &state->transientStorage.lightDir, SHADER_UNIFORM_VEC3);
}

EXPORT_FN void client_main(GameState *state) {
  init(state);
  time_t last_write_time = get_timestamp("./libclient.so");
  while (!WindowShouldClose()) {
    if (last_write_time != get_timestamp("./libclient.so"))
      break;
    state->frameCount++;
    state->deltaTime = GetFrameTime();
    update(state);
    render(state);
  }
  cleanup(state);
}
