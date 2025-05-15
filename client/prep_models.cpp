#include "raylib.h"
#include "utils.h"
#include "utils_client.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

// g++ prep_models.cpp -o prep_models -I ./libs -I ../libs -I ./src/ -I ./libs/raylib/src/ -I ./libs/rres/src/ -L ./libs/raylib/src/ -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Wl,-rpath,\$ORIGIN/ -fno-gnu-unique -Wno-format-security -g -O0

#ifdef _WIN32
  const char* RRESPACKER_PATH = ".\\libs\\rrespacker\\rrespacker.exe";
  const char* RESOURCE_CMD = "%s -o resources.rres --rrp resources.rrp";
#elif __linux__
  const char* RRESPACKER_PATH = "./libs/rrespacker/rrespacker";
  const char* RESOURCE_CMD = "%s -o resources.rres --rrp resources.rrp";
#elif __APPLE__
  const char* RRESPACKER_PATH = "./libs/rrespacker/rrespacker.app";
  const char* RESOURCE_CMD = "%s -o resources.rres --rrp resources.rrp";
#else
  #error "Unsupported platform"
#endif

#define MAX_MATERIAL_MAPS 12
#define RL_MAX_SHADER_LOCATIONS 32

Model& LoadModelFromChunkTest(const rresResourceChunk &chunk, Model &testModel, Arena& arena) {
  Model& model = arena.alloc<Model>();

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
  LOG_ASSERT(CompareMatrices(&model.transform, &testModel.transform, 1),
             "testModel and model don't match...");
  offset += sizeof(Matrix);

  // Read counts
  memcpy(&model.meshCount, data + offset, sizeof(int));
  LOG_ASSERT(model.meshCount == testModel.meshCount,
             "testModel and model don't match...");
  offset += sizeof(int);
  memcpy(&model.materialCount, data + offset, sizeof(int));
  LOG_ASSERT(model.materialCount == testModel.materialCount,
             "testModel and model don't match...");
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
    model.meshes = arena.alloc_raw<Mesh>(size);
    LOG_ASSERT(model.meshes != nullptr,
               "Failed to allocate memory for meshes: %zu bytes", size);

    for (int i = 0; i < model.meshCount; i++) {
      Mesh& mesh = model.meshes[i];

      // Initialize all mesh pointers to nullptr
      mesh.vertices = nullptr;
      mesh.texcoords = nullptr;
      mesh.texcoords2 = nullptr;
      mesh.normals = nullptr;
      mesh.tangents = nullptr;
      mesh.colors = nullptr;
      mesh.indices = nullptr;
      mesh.animVertices = nullptr;
      mesh.animNormals = nullptr;
      mesh.boneIds = nullptr;
      mesh.boneWeights = nullptr;
      mesh.boneMatrices = nullptr;
      mesh.vboId = nullptr;

      // Initialize all values to be 0
      mesh.vertexCount = 0;
      mesh.triangleCount = 0;
      mesh.vaoId = 0;
      mesh.boneCount = 0;

      // Read counts
      memcpy(&mesh.vertexCount, data + offset, sizeof(int));
      LOG_ASSERT(model.meshes[i].vertexCount ==
                     testModel.meshes[i].vertexCount,
                 "testModel and model don't match...");
      offset += sizeof(int);
      memcpy(&mesh.triangleCount, data + offset, sizeof(int));
      LOG_ASSERT(model.meshes[i].triangleCount ==
                     testModel.meshes[i].triangleCount,
                 "testModel and model don't match...");
      offset += sizeof(int);
      memcpy(&mesh.boneCount, data + offset, sizeof(int));
      LOG_ASSERT(model.meshes[i].boneCount == testModel.meshes[i].boneCount,
                 "testModel and model don't match...");
      offset += sizeof(int);

      unsigned char meshFlags;
      memcpy(&meshFlags, data + offset, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      unsigned char animFlags;
      memcpy(&animFlags, data + offset, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      // Read vertex data
      if (mesh.vertexCount > 0) {
        // Vertices
        if (meshFlags & 1) {
          size_t size = mesh.vertexCount * 3 * sizeof(float);
          mesh.vertices = arena.alloc_raw<float>(size);
          LOG_ASSERT(mesh.vertices != nullptr,
                     "Failed to allocate memory for vertices: %zu bytes", size);
          memcpy(mesh.vertices, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model.meshes[i].vertices,
                                        testModel.meshes[i].vertices, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Texcoords
        if (meshFlags & 2) {
          size_t size = mesh.vertexCount * 2 * sizeof(float);
          mesh.texcoords = arena.alloc_raw<float>(size);
          LOG_ASSERT(mesh.texcoords != nullptr,
                     "Failed to allocate memory for texcoords: %zu bytes",
                     size);
          memcpy(mesh.texcoords, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model.meshes[i].texcoords,
                                        testModel.meshes[i].texcoords, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Texcoords2
        if (meshFlags & 4) {
          size_t size = mesh.vertexCount * 2 * sizeof(float);
          mesh.texcoords2 = arena.alloc_raw<float>(size);
          LOG_ASSERT(mesh.texcoords2 != nullptr,
                     "Failed to allocate memory for texcoords2: %zu bytes",
                     size);
          memcpy(mesh.texcoords2, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model.meshes[i].texcoords2,
                                        testModel.meshes[i].texcoords2, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Normals
        if (meshFlags & 8) {
          size_t size = mesh.vertexCount * 3 * sizeof(float);
          mesh.normals = arena.alloc_raw<float>(size);
          LOG_ASSERT(mesh.normals != nullptr,
                     "Failed to allocate memory for normals: %zu bytes", size);
          memcpy(mesh.normals, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model.meshes[i].normals,
                                        testModel.meshes[i].normals, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Tangents
        if (meshFlags & 16) {
          size_t size = mesh.vertexCount * 4 * sizeof(float);
          mesh.tangents = arena.alloc_raw<float>(size);
          LOG_ASSERT(mesh.tangents != nullptr,
                     "Failed to allocate memory for tangents: %zu bytes", size);
          memcpy(mesh.tangents, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model.meshes[i].tangents,
                                        testModel.meshes[i].tangents, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Colors
        if (meshFlags & 32) {
          size_t size = mesh.vertexCount * 4 * sizeof(unsigned char *);
          mesh.colors = arena.alloc_raw<unsigned char>(size);
          LOG_ASSERT(mesh.colors != nullptr,
                     "Failed to allocate memory for colors: %zu bytes", size);
          memcpy(mesh.colors, data + offset, size);
          LOG_ASSERT(CompareUCharArrays(model.meshes[i].colors,
                                        testModel.meshes[i].colors, size),
                     "testModel and model don't match...");
          offset += size;
        }

        // Animation data
        // Animated vertices
        if (animFlags & 1) {
          size_t size = mesh.vertexCount * 3 * sizeof(float);
          mesh.animVertices = arena.alloc_raw<float>(size);
          LOG_ASSERT(mesh.animVertices != nullptr,
                     "Failed to allocate memory for anim vertices: %zu bytes",
                     size);
          memcpy(mesh.animVertices, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model.meshes[i].animVertices,
                                        testModel.meshes[i].animVertices, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Animated normals
        if (animFlags & 2) {
          size_t size = mesh.vertexCount * 3 * sizeof(float);
          mesh.animNormals = arena.alloc_raw<float>(size);
          LOG_ASSERT(mesh.animNormals != nullptr,
                     "Failed to allocate memory for anim normals: %zu bytes",
                     size);
          memcpy(mesh.animNormals, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model.meshes[i].animNormals,
                                        testModel.meshes[i].animNormals, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Bone IDs
        if (animFlags & 4) {
          size_t size = mesh.vertexCount * 4;
          mesh.boneIds = arena.alloc_raw<unsigned char>(size);
          LOG_ASSERT(mesh.boneIds != nullptr,
                     "Failed to allocate memory for bone IDs: %zu bytes", size);
          memcpy(mesh.boneIds, data + offset, size);
          LOG_ASSERT(CompareUCharArrays(model.meshes[i].boneIds,
                                        testModel.meshes[i].boneIds, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Bone weights
        if (animFlags & 8) {
          size_t size = mesh.vertexCount * 4 * sizeof(float);
          mesh.boneWeights = arena.alloc_raw<float>(size);
          LOG_ASSERT(mesh.boneWeights != nullptr,
                     "Failed to allocate memory for bone weights: %zu bytes",
                     size);
          memcpy(mesh.boneWeights, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model.meshes[i].boneWeights,
                                        testModel.meshes[i].boneWeights, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Bone matrices
        if (animFlags & 16 && mesh.boneCount > 0) {
          size_t size = mesh.boneCount * sizeof(Matrix);
          mesh.boneMatrices = arena.alloc_raw<Matrix>(size);
          LOG_ASSERT(mesh.boneMatrices != nullptr,
                     "Failed to allocate memory for bone matrices: %zu bytes",
                     size);
          memcpy(mesh.boneMatrices, data + offset, size);
          LOG_ASSERT(CompareMatrices(model.meshes[i].boneMatrices,
                                     testModel.meshes[i].boneMatrices,
                                     mesh.boneCount),
                     "testModel and model don't match...");
          offset += size;
        }
      }

      // Read indices
      if (mesh.triangleCount > 0 && (meshFlags & 64)) {
        size_t size = mesh.triangleCount * 3 * sizeof(unsigned short);
        mesh.indices = arena.alloc_raw<unsigned short>(size);
        LOG_ASSERT(mesh.indices != nullptr,
                   "Failed to allocate memory for indices: %zu bytes", size);
        memcpy(mesh.indices, data + offset, size);
        LOG_ASSERT(CompareUShortArrays(model.meshes[i].indices,
                                       testModel.meshes[i].indices, size),
                   "testModel and model don't match...");
        offset += size;
      }

      // Read OpenGL identifiers
      UploadMesh(&mesh, false);
    }
  }

  // Read materials
  if (globalFlags & 2) {
    int size = model.materialCount * sizeof(Material);
    model.materials = arena.alloc_raw<Material>(size);
    LOG_ASSERT(model.materials != nullptr,
               "Failed to allocate memory for materials: %zu bytes", size);

    for (int i = 0; i < model.materialCount; i++) {
      Material& material = model.materials[i];

      // Initialize material pointers
      material.shader.locs = nullptr;
      material.maps = nullptr;

      unsigned char matFlags;
      memcpy(&matFlags, data + offset, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      // Read shader ID
      memcpy(&material.shader.id, data + offset, sizeof(unsigned int));
      LOG_ASSERT(model.materials[i].shader.id ==
                     testModel.materials[i].shader.id,
                 "testModel and model don't match...");
      offset += sizeof(unsigned int);

      // Read shader locations
      if (matFlags & 1) {
        size_t size = RL_MAX_SHADER_LOCATIONS * sizeof(int);
        material.shader.locs = arena.alloc_raw<int>(size);
        LOG_ASSERT(material.shader.locs != nullptr,
                   "Failed to allocate memory for shader locations: %zu bytes",
                   size);
        memcpy(material.shader.locs, data + offset, size);
        LOG_ASSERT(CompareIntArrays(model.materials[i].shader.locs,
                                     testModel.materials[i].shader.locs, size),
                   "testModel and model don't match...");
        offset += size;
      }
      // Read material maps
      if (matFlags & 2) {
        size_t size = MAX_MATERIAL_MAPS * sizeof(MaterialMap);
        material.maps = arena.alloc_raw<MaterialMap>(size);
        LOG_ASSERT(material.maps != nullptr,
                   "Failed to allocate memory for material maps: %zu bytes",
                   size);

        // Read each material map
        for (int j = 0; j < MAX_MATERIAL_MAPS; j++) {
          // Read texture
          memcpy(&material.maps[j].texture, data + offset, sizeof(Texture));
          LOG_ASSERT(CompareTexture(model.materials[i].maps[j].texture,
                                    testModel.materials[i].maps[j].texture),
                     "testModel and model don't match...");
          offset += sizeof(Texture);

          // Read color
          memcpy(&material.maps[j].color, data + offset, sizeof(Color));
          LOG_ASSERT(CompareColor(model.materials[i].maps[j].color,
                                  testModel.materials[i].maps[j].color),
                     "testModel and model don't match...");
          offset += sizeof(Color);

          // Read value
          memcpy(&material.maps[j].value, data + offset, sizeof(float));
          LOG_ASSERT(model.materials[i].maps[j].value ==
                         testModel.materials[i].maps[j].value,
                     "testModel and model don't match...");
          offset += sizeof(float);
        }
      }

      // Read material params (all 4 floats)
      memcpy(&material.params, data + offset, sizeof(float) * 4);
      LOG_ASSERT(CompareFloatArrays(model.materials[i].params,
                                    testModel.materials[i].params,
                                    sizeof(float) * 4),
                 "testModel and model don't match...");
      offset += sizeof(float) * 4;
    }
  }

  // Read mesh material indices
  if (globalFlags & 4) {
    int size = model.meshCount * sizeof(int);
    model.meshMaterial = arena.alloc_raw<int>(size);
    LOG_ASSERT(model.meshMaterial != nullptr,
               "Failed to allocate memory for mesh materials: %zu bytes", size);
    memcpy(model.meshMaterial, data + offset, size);
    LOG_ASSERT(
        CompareIntArrays(model.meshMaterial, testModel.meshMaterial, size),
        "testModel and model don't match...");
    offset += size;
  }

  memcpy(&model.boneCount, data + offset, sizeof(int));
  LOG_ASSERT(model.boneCount == testModel.boneCount,
             "testModel and model don't match...");
  offset += sizeof(int);

  if (model.boneCount > 0) {
    // Read bones
    if (globalFlags & 8) {
      int size = model.boneCount * sizeof(BoneInfo);
      model.bones = arena.alloc_raw<BoneInfo>(size);
      LOG_ASSERT(model.bones != nullptr,
                 "Failed to allocate memory for bones: %zu bytes", size);
      memcpy(model.bones, data + offset, size);
      LOG_ASSERT(CompareBones(model.bones, testModel.bones, size),
                 "testModel and model don't match...");
      offset += size;
    }

    // Read bind pose
    if (globalFlags & 16) {
      int size = model.boneCount * sizeof(Transform);
      model.bindPose = arena.alloc_raw<Transform>(size);
      LOG_ASSERT(model.bindPose != nullptr,
                 "Failed to allocate memory for bind pose: %zu bytes", size);

      for (int i = 0; i < model.boneCount; i++) {
        memcpy(&model.bindPose[i].translation, data + offset, sizeof(Vector3));
        LOG_ASSERT(CompareVector3(model.bindPose[i].translation,
                                  testModel.bindPose[i].translation),
                   "testModel and model don't match...");
        offset += sizeof(Vector3);
        memcpy(&model.bindPose[i].rotation, data + offset, sizeof(Vector4));
        LOG_ASSERT(CompareVector4(model.bindPose[i].rotation,
                                  testModel.bindPose[i].rotation),
                   "testModel and model don't match...");
        offset += sizeof(Vector4);
        memcpy(&model.bindPose[i].scale, data + offset, sizeof(Vector3));
        LOG_ASSERT(CompareVector3(model.bindPose[i].scale,
                                  testModel.bindPose[i].scale),
                   "testModel and model don't match...");
        offset += sizeof(Vector3);
      }
    }
  }

  return model;
}

void ExportModelToBinary(const Model &model, const char *filename, Arena& arena) {
  if (!filename)
    return;

  char* buffer = arena.alloc_raw<char>(KB(50));
  char* current = buffer;

  // Write transform matrix
  memcpy(current, &model.transform, sizeof(Matrix));
  current += sizeof(Matrix);
  LOG_TRACE("After transform matrix: %zu bytes\n", current - buffer);

  // Write counts
  memcpy(current, &model.meshCount, sizeof(int));
  current += sizeof(int);
  LOG_TRACE("After mesh count: %zu bytes\n", current - buffer);
  memcpy(current, &model.materialCount, sizeof(int));
  current += sizeof(int);
  LOG_TRACE("After material count: %zu bytes\n", current - buffer);

  // Write global flags
  unsigned char globalFlags = 0;
  globalFlags |= (model.meshes ? 1 : 0);
  globalFlags |= (model.materials ? 2 : 0);
  globalFlags |= (model.meshMaterial ? 4 : 0);
  globalFlags |= (model.bones ? 8 : 0);
  globalFlags |= (model.bindPose ? 16 : 0);
  memcpy(current, &globalFlags, sizeof(unsigned char));
  current += sizeof(unsigned char);
  LOG_TRACE("After global flags: %zu bytes\n", current - buffer);

  // Write meshes
  if (model.meshes) {
    for (int i = 0; i < model.meshCount; i++) {
      const Mesh &mesh = model.meshes[i];

      // Write counts
      memcpy(current, &mesh.vertexCount, sizeof(int));
      current += sizeof(int);
      LOG_TRACE("After mesh %i vertex count: %zu bytes\n", i, current - buffer);
      memcpy(current, &mesh.triangleCount, sizeof(int));
      current += sizeof(int);
      LOG_TRACE("After mesh %i triangle count: %zu bytes\n", i, current - buffer);
      memcpy(current, &mesh.boneCount, sizeof(int));
      current += sizeof(int);
      LOG_TRACE("After mesh %i bone count: %zu bytes\n", i, current - buffer);

      // Write mesh flags
      unsigned char meshFlags = 0;
      meshFlags |= (mesh.vertices ? 1 : 0);
      meshFlags |= (mesh.texcoords ? 2 : 0);
      meshFlags |= (mesh.texcoords2 ? 4 : 0);
      meshFlags |= (mesh.normals ? 8 : 0);
      meshFlags |= (mesh.tangents ? 16 : 0);
      meshFlags |= (mesh.colors ? 32 : 0);
      meshFlags |= (mesh.indices ? 64 : 0);
      memcpy(current, &meshFlags, sizeof(unsigned char));
      current += sizeof(unsigned char);
      LOG_TRACE("After mesh %i mesh flags: %zu bytes\n", i, current - buffer);

      // Write animation flags
      unsigned char animFlags = 0;
      animFlags |= (mesh.animVertices ? 1 : 0);
      animFlags |= (mesh.animNormals ? 2 : 0);
      animFlags |= (mesh.boneIds ? 4 : 0);
      animFlags |= (mesh.boneWeights ? 8 : 0);
      animFlags |= (mesh.boneMatrices ? 16 : 0);
      memcpy(current, &animFlags, sizeof(unsigned char));
      current += sizeof(unsigned char);
      LOG_TRACE("After mesh %i animation flags: %zu bytes\n", i, current - buffer);

      // Write vertex data
      if (mesh.vertexCount > 0) {
        if (mesh.vertices) {
          size_t size = sizeof(float) * mesh.vertexCount * 3;
          memcpy(current, mesh.vertices, size);
          current += size;
          LOG_TRACE("After mesh %i vertices: %zu bytes\n", i, current - buffer);
        }
        if (mesh.texcoords) {
          size_t size = sizeof(float) * mesh.vertexCount * 2;
          memcpy(current, mesh.texcoords, size);
          current += size;
          LOG_TRACE("After mesh %i texcoords: %zu bytes\n", i, current - buffer);
        }
        if (mesh.texcoords2) {
          size_t size = sizeof(float) * mesh.vertexCount * 2;
          memcpy(current, mesh.texcoords2, size);
          current += size;
          LOG_TRACE("After mesh %i texcoords2: %zu bytes\n", i, current - buffer);
        }
        if (mesh.normals) {
          size_t size = sizeof(float) * mesh.vertexCount * 3;
          memcpy(current, mesh.normals, size);
          current += size;
          LOG_TRACE("After mesh %i normals: %zu bytes\n", i, current - buffer);
        }
        if (mesh.tangents) {
          size_t size = sizeof(float) * mesh.vertexCount * 4;
          memcpy(current, mesh.tangents, size);
          current += size;
          LOG_TRACE("After mesh %i tangents: %zu bytes\n", i, current - buffer);
        }
        if (mesh.colors) {
          size_t size = sizeof(unsigned char) * mesh.vertexCount * 4;
          memcpy(current, mesh.colors, size);
          current += size;
          LOG_TRACE("After mesh %i colors: %zu bytes\n", i, current - buffer);
        }

        // Write animation data
        if (mesh.animVertices) {
          size_t size = sizeof(float) * mesh.vertexCount * 3;
          memcpy(current, mesh.animVertices, size);
          current += size;
          LOG_TRACE("After mesh %i anim vertices: %zu bytes\n", i, current - buffer);
        }
        if (mesh.animNormals) {
          size_t size = sizeof(float) * mesh.vertexCount * 3;
          memcpy(current, mesh.animNormals, size);
          current += size;
          LOG_TRACE("After mesh %i anim normals: %zu bytes\n", i, current - buffer);
        }
        if (mesh.boneIds) {
          size_t size = sizeof(unsigned char) * mesh.vertexCount * 4;
          memcpy(current, mesh.boneIds, size);
          current += size;
          LOG_TRACE("After mesh %i bone IDs: %zu bytes\n", i, current - buffer);
        }
        if (mesh.boneWeights) {
          size_t size = sizeof(float) * mesh.vertexCount * 4;
          memcpy(current, mesh.boneWeights, size);
          current += size;
          LOG_TRACE("After mesh %i bone weights: %zu bytes\n", i, current - buffer);
        }
        if (mesh.boneMatrices && mesh.boneCount > 0) {
          size_t size = sizeof(Matrix) * mesh.boneCount;
          memcpy(current, mesh.boneMatrices, size);
          current += size;
          LOG_TRACE("After mesh %i boneMatrices: %zu bytes\n", i, current - buffer);
        }
      }

      // Write indices
      if (mesh.triangleCount > 0 && mesh.indices) {
        size_t size = sizeof(unsigned short) * mesh.triangleCount * 3;
        memcpy(current, mesh.indices, size);
        current += size;
        LOG_TRACE("After mesh %i indices: %zu bytes\n", i, current - buffer);
      }
    }
  }

  // Write materials
  if (model.materials) {
    for (int i = 0; i < model.materialCount; i++) {
      const Material &material = model.materials[i];

      // Write material flags
      unsigned char matFlags = 0;
      matFlags |= (material.shader.locs ? 1 : 0);
      matFlags |= (material.maps ? 2 : 0);
      memcpy(current, &matFlags, sizeof(unsigned char));
      current += sizeof(unsigned char);
      LOG_TRACE("After material %i material flags: %zu bytes\n", i, current - buffer);

      // Write shader
      memcpy(current, &material.shader.id, sizeof(unsigned int));
      current += sizeof(unsigned int);
      LOG_TRACE("After material %i shader id: %zu bytes\n", i, current - buffer);

      if (material.shader.locs) {
        size_t size = sizeof(int) * RL_MAX_SHADER_LOCATIONS;
        memcpy(current, material.shader.locs, size);
        current += size;
        LOG_TRACE("After material %i shader locs: %zu bytes\n", i, current - buffer);
      }

      // Write material maps
      if (material.maps) {
        for (int j = 0; j < MAX_MATERIAL_MAPS; j++) {
          const MaterialMap &map = material.maps[j];
          memcpy(current, &map.texture, sizeof(Texture));
          current += sizeof(Texture);
          LOG_TRACE("After material %i map %i texture: %zu bytes\n", i, j, current - buffer);
          memcpy(current, &map.color, sizeof(Color));
          current += sizeof(Color);
          LOG_TRACE("After material %i map %i color: %zu bytes\n", i, j, current - buffer);
          memcpy(current, &map.value, sizeof(float));
          current += sizeof(float);
          LOG_TRACE("After material %i map %i value: %zu bytes\n", i, j, current - buffer);
        }
      }

      // Write material parameters
      memcpy(current, material.params, sizeof(float) * 4);
      current += sizeof(float) * 4;
      LOG_TRACE("After material %i params: %zu bytes\n", i, current - buffer);
    }
  }

  // Write mesh material indices
  if (model.meshMaterial) {
    size_t size = sizeof(int) * model.meshCount;
    memcpy(current, model.meshMaterial, size);
    current += size;
    LOG_TRACE("After mesh material: %zu bytes\n", current - buffer);
  }

  // BoneCount
  memcpy(current, &model.boneCount, sizeof(int));
  current += sizeof(int);
  LOG_TRACE("After bone count: %zu bytes\n", current - buffer);

  if (model.boneCount > 0) {
    // BoneInfo
    if (model.bones) {
      size_t size = sizeof(BoneInfo) * model.boneCount;
      memcpy(current, model.bones, size);
      current += size;
      LOG_TRACE("After bone info: %zu bytes\n", current - buffer);
    }

    // bindPose
    if (model.bindPose) {
      for (int i = 0; i < model.boneCount; i++) {
        memcpy(current, &model.bindPose[i].translation, sizeof(Vector3));
        current += sizeof(Vector3);
        LOG_TRACE("After bind pose %i translation: %zu bytes\n", i, current - buffer);
        memcpy(current, &model.bindPose[i].rotation, sizeof(Vector4));
        current += sizeof(Vector4);
        LOG_TRACE("After bind pose %i rotation: %zu bytes\n", i, current - buffer);
        memcpy(current, &model.bindPose[i].scale, sizeof(Vector3));
        current += sizeof(Vector3);
        LOG_TRACE("After bind pose %i scale: %zu bytes\n", i, current - buffer);
      }
    }
  }

  write_file(filename, buffer, current - buffer);
}

ArrayCT<const char*, 100>& listFiles(const char* path, Arena& arena) {
  DIR* dir;
  struct dirent *entry;
  ArrayCT<const char*, 100>& files = arena.create_array_ct<const char*, 100>();

  dir = opendir(path);

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      if (strstr(entry->d_name, ".glb")) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        files.add(strdup(fullpath));
      }
    } else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 &&
               strcmp(entry->d_name, "..") != 0) {
      char subpath[1024];
      snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);

      ArrayCT<const char*, 100>& subfiles = listFiles(subpath, arena);

      for (uint32_t i = 0; i < subfiles.size(); i++) {
        files.add(subfiles[i]);
      }
    }
  }

  closedir(dir);
  return files;
}

bool PackResources() {
  // Check if rrespacker exists
  FILE* test = fopen(RRESPACKER_PATH, "r");
  if (!test) {
    LOG_ERROR("Error: rrespacker not found at %s\n", RRESPACKER_PATH);
    return false;
  }
  fclose(test);

  // Create command
  char cmd[512];
  snprintf(cmd, sizeof(cmd), RESOURCE_CMD, RRESPACKER_PATH);

  // Execute command
  int result = system(cmd);
  return result == 0;
}

int main(int argc, char *argv[]) {
  InitWindow(800, 450, "prep models");

  Arena& arena = * new Arena(MB(1));
  MapCT<const char*, Model, 100>& modelMap = arena.create_map_ct<const char*, Model, 100>();

  ArrayCT<const char*, 100>& models = listFiles("resources/models", arena);

  const char* OUTPUT_DIR = "./resources/models/";
  char out_path[256];

  for (uint32_t i = 0; i < models.size(); i++) {
    const char *in = models[i];
    const char *filename = strrchr(in, '/');
    filename = filename ? filename + 1 : in;

    // Create binary filename without .obj extension
    char bin_filename[256];
    snprintf(bin_filename, sizeof(bin_filename), "%.*s.bin", (int)(strlen(filename) - 4), filename);

    // Construct full output path
    size_t remaining = sizeof(out_path);
    strncpy(out_path, OUTPUT_DIR, remaining);
    remaining -= strlen(OUTPUT_DIR);
    strncat(out_path, bin_filename, remaining - 1);

    LOG_TRACE("%s -> %s", in, out_path);
    Model model = LoadModel(in);
    ExportModelToBinary(model, out_path, arena);

    // Store in map with persistent key
    size_t key_len = strlen(bin_filename) + 1;
    char* persistent_key = arena.alloc_count_raw<char>(key_len);
    strcpy(persistent_key, bin_filename);
    modelMap[persistent_key] = model;
  }

  if (!PackResources()) {
    LOG_ERROR("Failed to pack resources\n");
    return 1;
  }

  // Testing
  rresCentralDir dir = rresLoadCentralDirectory("resources.rres");
  for (auto &[path, testModel] : modelMap) {
    int idModel = rresGetResourceId(dir, path);
    rresResourceChunk chunkModel = rresLoadResourceChunk("resources.rres", idModel);
    Model& modelTest = LoadModelFromChunkTest(chunkModel, testModel, arena);
    UnloadModel(testModel);
    rresUnloadResourceChunk(chunkModel);
  }

  rresUnloadCentralDirectory(dir);
  delete &arena;

  return 0;
}
