#include "entt.hpp"
#include "game_state.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "utils.h"

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
#define MAX_MATERIAL_MAPS 12
#define RL_MAX_SHADER_LOCATIONS 32

void render(GameState *state) {
  BeginDrawing();

  Matrix lightView;
  Matrix lightProj;
  BeginTextureMode(state->transientStorage.shadowMap);
  ClearBackground(WHITE);
  BeginMode3D(state->lightCamera);
  lightView = rlGetMatrixModelview();
  lightProj = rlGetMatrixProjection();

  DrawModelEx(state->transientStorage.resourceManager.roverAssets.body, state->transientStorage.resourceManager.roverAssets.bodyTranlation, (Vector3){0.0f, 1.0f, 0.0f},
              0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.blWheelTranslation, (Vector3){0.0f, 1.0f, 0.0f},
              0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.brWheelTranslation, (Vector3){0.0f, 1.0f, 0.0f},
              0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.flWheelTranslation, (Vector3){0.0f, 1.0f, 0.0f},
              0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.frWheelTranslation, (Vector3){0.0f, 1.0f, 0.0f},
              0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
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
  rlSetUniform(state->transientStorage.shadowMapLoc, &slot, SHADER_UNIFORM_INT,
               1);

  BeginMode3D(state->camera);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.scan, state->transientStorage.resourceManager.roverAssets.scanTranslation, (Vector3){0.0f, 1.0f, 0.0f},
              0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.body, state->transientStorage.resourceManager.roverAssets.bodyTranlation, (Vector3){0.0f, 1.0f, 0.0f},
              0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.blWheelTranslation, (Vector3){0.0f, 1.0f, 0.0f},
              0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.brWheelTranslation, (Vector3){0.0f, 1.0f, 0.0f},
              0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.flWheelTranslation, (Vector3){0.0f, 1.0f, 0.0f},
              0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
  DrawModelEx(state->transientStorage.resourceManager.roverAssets.wheel, state->transientStorage.resourceManager.roverAssets.frWheelTranslation, (Vector3){0.0f, 1.0f, 0.0f},
              0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
  EndMode3D();

  DrawFPS(10, 10);

  DrawText("Use key [Y] to toggle lights", 10, 40, 20,
           GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)));

  GuiGui(&state->gui, &state->dir);
  EndDrawing();
}

Model LoadModelFromChunk(const rresResourceChunk &chunk) {
  Model model = {0};

  if (!chunk.data.raw) {
    LOG_ERROR("Chunk data is null");
    return model;
  }

  // Initialize all pointers to nullptr explicitly
  model.meshes = nullptr;
  model.materials = nullptr;
  model.meshMaterial = nullptr;
  model.bones = nullptr;
  model.bindPose = nullptr;

  const unsigned char *data = (const unsigned char *)chunk.data.raw;
  size_t offset = 0;

  // Read transform matrix
  memcpy(&model.transform, data + offset, sizeof(Matrix));
  offset += sizeof(Matrix);

  // Read counts
  memcpy(&model.meshCount, data + offset, sizeof(int));
  offset += sizeof(int);
  memcpy(&model.materialCount, data + offset, sizeof(int));
  offset += sizeof(int);

  LOG_TRACE("Loading model with %d meshes and %d materials", model.meshCount,
            model.materialCount);

  // Read global flags
  unsigned char globalFlags;
  memcpy(&globalFlags, data + offset, sizeof(unsigned char));
  offset += sizeof(unsigned char);

  // Read meshes
  if (globalFlags & 1) {
    int size = model.meshCount * sizeof(Mesh);
    model.meshes = (Mesh *)malloc(size);

    for (int i = 0; i < model.meshCount; i++) {
      Mesh *mesh = &model.meshes[i];

      // Initialize all mesh pointers to nullptr
      mesh->vertices = nullptr;
      mesh->texcoords = nullptr;
      mesh->texcoords2 = nullptr;
      mesh->normals = nullptr;
      mesh->tangents = nullptr;
      mesh->colors = nullptr;
      mesh->indices = nullptr;
      mesh->animVertices = nullptr;
      mesh->animNormals = nullptr;
      mesh->boneIds = nullptr;
      mesh->boneWeights = nullptr;
      mesh->boneMatrices = nullptr;
      mesh->vboId = nullptr;

      // Initialize all values to be 0
      mesh->vertexCount = 0;
      mesh->triangleCount = 0;
      mesh->vaoId = 0;
      mesh->boneCount = 0;

      // Read counts
      memcpy(&mesh->vertexCount, data + offset, sizeof(int));
      offset += sizeof(int);
      memcpy(&mesh->triangleCount, data + offset, sizeof(int));
      offset += sizeof(int);
      memcpy(&mesh->boneCount, data + offset, sizeof(int));
      offset += sizeof(int);

      unsigned char meshFlags;
      memcpy(&meshFlags, data + offset, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      unsigned char animFlags;
      memcpy(&animFlags, data + offset, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      // Read vertex data
      if (mesh->vertexCount > 0) {
        // Vertices
        if (meshFlags & 1) {
          size_t size = mesh->vertexCount * 3 * sizeof(float);
          mesh->vertices = (float *)malloc(size);
          memcpy(mesh->vertices, data + offset, size);
          offset += size;
        }
        // Texcoords
        if (meshFlags & 2) {
          size_t size = mesh->vertexCount * 2 * sizeof(float);
          mesh->texcoords = (float *)malloc(size);
          memcpy(mesh->texcoords, data + offset, size);
          offset += size;
        }
        // Texcoords2
        if (meshFlags & 4) {
          size_t size = mesh->vertexCount * 2 * sizeof(float);
          mesh->texcoords2 = (float *)malloc(size);
          memcpy(mesh->texcoords2, data + offset, size);
          offset += size;
        }
        // Normals
        if (meshFlags & 8) {
          size_t size = mesh->vertexCount * 3 * sizeof(float);
          mesh->normals = (float *)malloc(size);
          memcpy(mesh->normals, data + offset, size);
          offset += size;
        }
        // Tangents
        if (meshFlags & 16) {
          size_t size = mesh->vertexCount * 4 * sizeof(float);
          mesh->tangents = (float *)malloc(size);
          memcpy(mesh->tangents, data + offset, size);
          offset += size;
        }
        // Colors
        if (meshFlags & 32) {
          size_t size = mesh->vertexCount * 4 * sizeof(unsigned char *);
          mesh->colors = (unsigned char *)malloc(size);
          memcpy(mesh->colors, data + offset, size);
          offset += size;
        }

        // Animation data
        // Animated vertices
        if (animFlags & 1) {
          size_t size = mesh->vertexCount * 3 * sizeof(float);
          mesh->animVertices = (float *)malloc(size);
          memcpy(mesh->animVertices, data + offset, size);
          offset += size;
        }
        // Animated normals
        if (animFlags & 2) {
          size_t size = mesh->vertexCount * 3 * sizeof(float);
          mesh->animNormals = (float *)malloc(size);
          memcpy(mesh->animNormals, data + offset, size);
          offset += size;
        }
        // Bone IDs
        if (animFlags & 4) {
          size_t size = mesh->vertexCount * 4;
          mesh->boneIds = (unsigned char *)malloc(size);
          memcpy(mesh->boneIds, data + offset, size);
          offset += size;
        }
        // Bone weights
        if (animFlags & 8) {
          size_t size = mesh->vertexCount * 4 * sizeof(float);
          mesh->boneWeights = (float *)malloc(size);
          memcpy(mesh->boneWeights, data + offset, size);
          offset += size;
        }
        // Bone matrices
        if (animFlags & 16 && mesh->boneCount > 0) {
          size_t size = mesh->boneCount * sizeof(Matrix);
          mesh->boneMatrices = (Matrix *)malloc(size);
          memcpy(mesh->boneMatrices, data + offset, size);
          offset += size;
        }
      }

      // Read indices
      if (mesh->triangleCount > 0 && (meshFlags & 64)) {
        size_t size = mesh->triangleCount * 3 * sizeof(unsigned short);
        mesh->indices = (unsigned short *)malloc(size);
        memcpy(mesh->indices, data + offset, size);
        offset += size;
      }

      // Read OpenGL identifiers
      UploadMesh(mesh, false);
    }
  }

  // Read materials
  if (globalFlags & 2) {
    int size = model.materialCount * sizeof(Material);
    model.materials = (Material *)malloc(size);

    for (int i = 0; i < model.materialCount; i++) {
      Material *material = &model.materials[i];

      // Initialize material pointers
      material->shader.locs = nullptr;
      material->maps = nullptr;

      unsigned char matFlags;
      memcpy(&matFlags, data + offset, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      // Read shader ID
      memcpy(&material->shader.id, data + offset, sizeof(unsigned int));
      offset += sizeof(unsigned int);

      // Read shader locations
      if (matFlags & 1) {
        size_t size = RL_MAX_SHADER_LOCATIONS * sizeof(int);
        material->shader.locs = (int *)malloc(size);
        memcpy(material->shader.locs, data + offset, size);
        offset += size;
      }
      // Read material maps
      if (matFlags & 2) {
        size_t size = MAX_MATERIAL_MAPS * sizeof(MaterialMap);
        material->maps = (MaterialMap *)malloc(size);

        // Read each material map
        for (int j = 0; j < MAX_MATERIAL_MAPS; j++) {
          // Read texture
          memcpy(&material->maps[j].texture, data + offset, sizeof(Texture));
          offset += sizeof(Texture);

          // Read color
          memcpy(&material->maps[j].color, data + offset, sizeof(Color));
          offset += sizeof(Color);

          // Read value
          memcpy(&material->maps[j].value, data + offset, sizeof(float));
          offset += sizeof(float);
        }
      }

      // Read material params (all 4 floats)
      memcpy(&material->params, data + offset, sizeof(float) * 4);
      offset += sizeof(float) * 4;
    }
  }

  // Read mesh material indices
  if (globalFlags & 4) {
    int size = model.meshCount * sizeof(int);
    model.meshMaterial = (int *)malloc(size);
    memcpy(model.meshMaterial, data + offset, size);
    offset += size;
  }

  memcpy(&model.boneCount, data + offset, sizeof(int));
  offset += sizeof(int);

  if (model.boneCount > 0) {
    // Read bones
    if (globalFlags & 8) {
      int size = model.boneCount * sizeof(BoneInfo);
      model.bones = (BoneInfo *)malloc(size);
      memcpy(model.bones, data + offset, size);
      offset += size;
    }

    // Read bind pose
    if (globalFlags & 16) {
      int size = model.boneCount * sizeof(Transform);
      model.bindPose = (Transform *)malloc(size);

      for (int i = 0; i < model.boneCount; i++) {
        memcpy(&model.bindPose[i].translation, data + offset, sizeof(Vector3));
        offset += sizeof(Vector3);
        memcpy(&model.bindPose[i].rotation, data + offset, sizeof(Vector4));
        offset += sizeof(Vector4);
        memcpy(&model.bindPose[i].scale, data + offset, sizeof(Vector3));
        offset += sizeof(Vector3);
      }
    }
  }

  return model;
}

void init(GameState *state) {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(800, 450, "video game");
  SetTargetFPS(120);

  // Hack used to check if this current load is a hot code reload
  bool isEcsDeinitialized = state->registry.storage<entt::entity>().empty();

  state->dir = rresLoadCentralDirectory("resources.rres");

  int idRoverBody = rresGetResourceId(state->dir, "rover_body.bin");
  rresResourceChunk chunkRoverBody = rresLoadResourceChunk("resources.rres", idRoverBody);
  state->transientStorage.resourceManager.roverAssets.body = LoadModelFromChunk(chunkRoverBody);

  int idRoverScan = rresGetResourceId(state->dir, "rover_scan.bin");
  rresResourceChunk chunkRoverScan = rresLoadResourceChunk("resources.rres", idRoverScan);
  state->transientStorage.resourceManager.roverAssets.scan = LoadModelFromChunk(chunkRoverScan);

  int idRoverWheel = rresGetResourceId(state->dir, "rover_wheel.bin");
  rresResourceChunk chunkRoverWheel = rresLoadResourceChunk("resources.rres", idRoverWheel);
  state->transientStorage.resourceManager.roverAssets.wheel = LoadModelFromChunk(chunkRoverWheel);

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

  if (isEcsDeinitialized) {
    state->camera.position = (Vector3){10.0f, 10.0f, 10.0f};
    state->camera.target = Vector3Zero();
    state->camera.projection = CAMERA_PERSPECTIVE;
    state->camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    state->camera.fovy = 45.0f;
  }

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

  if (isEcsDeinitialized) {
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

extern "C" void client_main(GameState *state) {
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
