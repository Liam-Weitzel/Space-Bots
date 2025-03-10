#include "raylib.h"
#include "utils.h"
#include <cmath>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#define RRES_IMPLEMENTATION
#include "rres.h"

#define RRES_RAYLIB_IMPLEMENTATION
#include "rres-raylib.h"

// g++ prep_models.cpp -o prep_models -I ./src/ -I ./libs/raylib/src/ -I
// ./libs/rres/src/ -L ./libs/raylib/src/ -lraylib -lGL -lm -lpthread -ldl -lrt
// -lX11 -Wl,-rpath,\$ORIGIN/ -fno-gnu-unique -Wno-format-security -g -O0

#define MAX_MATERIAL_MAPS 12
#define RL_MAX_SHADER_LOCATIONS 32

bool CompareFloat(float a, float b, float epsilon = 0.0001f) {
  return fabs(a - b) <= epsilon;
}

bool CompareVector3(const Vector3 &a, const Vector3 &b,
                    float epsilon = 0.0001f) {
  return CompareFloat(a.x, b.x, epsilon) && CompareFloat(a.y, b.y, epsilon) &&
         CompareFloat(a.z, b.z, epsilon);
}

bool CompareVector4(const Vector4 &a, const Vector4 &b,
                    float epsilon = 0.0001f) {
  return CompareFloat(a.x, b.x, epsilon) && CompareFloat(a.y, b.y, epsilon) &&
         CompareFloat(a.z, b.z, epsilon) && CompareFloat(a.w, b.w, epsilon);
}

bool CompareShaderLocs(const int *a, const int *b, size_t size) {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;
  return memcmp(a, b, size) == 0;
}

bool CompareMeshMaterials(const int *a, const int *b, size_t size) {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;
  return memcmp(a, b, size) == 0;
}

bool CompareBones(const BoneInfo *a, const BoneInfo *b, size_t size) {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;
  return memcmp(a, b, size) == 0;
}

bool CompareFloatArrays(const float *a, const float *b, size_t size) {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;
  return memcmp(a, b, size) == 0;
}

bool CompareUCharArrays(const unsigned char *a, const unsigned char *b,
                        size_t size) {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;
  return memcmp(a, b, size) == 0;
}

bool CompareMatrices(const Matrix *a, const Matrix *b, size_t count) {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;

  for (size_t i = 0; i < count; i++) {
    if (a[i].m0 != b[i].m0 || a[i].m1 != b[i].m1 || a[i].m2 != b[i].m2 ||
        a[i].m3 != b[i].m3 || a[i].m4 != b[i].m4 || a[i].m5 != b[i].m5 ||
        a[i].m6 != b[i].m6 || a[i].m7 != b[i].m7 || a[i].m8 != b[i].m8 ||
        a[i].m9 != b[i].m9 || a[i].m10 != b[i].m10 || a[i].m11 != b[i].m11 ||
        a[i].m12 != b[i].m12 || a[i].m13 != b[i].m13 || a[i].m14 != b[i].m14 ||
        a[i].m15 != b[i].m15) {
      return false;
    }
  }
  return true;
}

bool CompareColor(const Color &a, const Color &b) {
  return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

bool CompareTexture(const Texture &a, const Texture &b) {
  return a.id == b.id && a.width == b.width && a.height == b.height &&
         a.format == b.format && a.mipmaps == b.mipmaps;
}

bool CompareUIntArrays(const unsigned int *a, const unsigned int *b,
                       size_t size) {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;
  return memcmp(a, b, size) == 0;
}

bool CompareUShortArrays(const unsigned short *a, const unsigned short *b,
                         size_t size) {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;
  return memcmp(a, b, size) == 0;
}

Model *LoadModelFromChunk(const rresResourceChunk &chunk, Model &testModel,
                          Arena* arena) {
  Model *model = arena->alloc<Model>();

  if (!chunk.data.raw) {
    LOG_ERROR("Chunk data is null");
    return model;
  }

  // Initialize all pointers to nullptr explicitly
  model->meshes = nullptr;
  model->materials = nullptr;
  model->meshMaterial = nullptr;
  model->bones = nullptr;
  model->bindPose = nullptr;

  const unsigned char *data = (const unsigned char *)chunk.data.raw;
  size_t offset = 0;

  // Read transform matrix
  memcpy(&model->transform, data + offset, sizeof(Matrix));
  LOG_ASSERT(CompareMatrices(&model->transform, &testModel.transform, 1),
             "testModel and model don't match...");
  offset += sizeof(Matrix);

  // Read counts
  memcpy(&model->meshCount, data + offset, sizeof(int));
  LOG_ASSERT(model->meshCount == testModel.meshCount,
             "testModel and model don't match...");
  offset += sizeof(int);
  memcpy(&model->materialCount, data + offset, sizeof(int));
  LOG_ASSERT(model->materialCount == testModel.materialCount,
             "testModel and model don't match...");
  offset += sizeof(int);

  LOG_TRACE("Loading model with %d meshes and %d materials", model->meshCount,
            model->materialCount);

  // Read global flags
  unsigned char globalFlags;
  memcpy(&globalFlags, data + offset, sizeof(unsigned char));
  offset += sizeof(unsigned char);

  // Read meshes
  if (globalFlags & 1) {
    int size = model->meshCount * sizeof(Mesh);
    model->meshes = arena->alloc<Mesh>(size);
    LOG_ASSERT(model->meshes != nullptr,
               "Failed to allocate memory for meshes: %zu bytes", size);

    for (int i = 0; i < model->meshCount; i++) {
      Mesh *mesh = &model->meshes[i];

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
      LOG_ASSERT(model->meshes[i].vertexCount ==
                     testModel.meshes[i].vertexCount,
                 "testModel and model don't match...");
      offset += sizeof(int);
      memcpy(&mesh->triangleCount, data + offset, sizeof(int));
      LOG_ASSERT(model->meshes[i].triangleCount ==
                     testModel.meshes[i].triangleCount,
                 "testModel and model don't match...");
      offset += sizeof(int);
      memcpy(&mesh->boneCount, data + offset, sizeof(int));
      LOG_ASSERT(model->meshes[i].boneCount == testModel.meshes[i].boneCount,
                 "testModel and model don't match...");
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
          mesh->vertices = arena->alloc<float>(size);
          LOG_ASSERT(mesh->vertices != nullptr,
                     "Failed to allocate memory for vertices: %zu bytes", size);
          memcpy(mesh->vertices, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model->meshes[i].vertices,
                                        testModel.meshes[i].vertices, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Texcoords
        if (meshFlags & 2) {
          size_t size = mesh->vertexCount * 2 * sizeof(float);
          mesh->texcoords = arena->alloc<float>(size);
          LOG_ASSERT(mesh->texcoords != nullptr,
                     "Failed to allocate memory for texcoords: %zu bytes",
                     size);
          memcpy(mesh->texcoords, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model->meshes[i].texcoords,
                                        testModel.meshes[i].texcoords, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Texcoords2
        if (meshFlags & 4) {
          size_t size = mesh->vertexCount * 2 * sizeof(float);
          mesh->texcoords2 = arena->alloc<float>(size);
          LOG_ASSERT(mesh->texcoords2 != nullptr,
                     "Failed to allocate memory for texcoords2: %zu bytes",
                     size);
          memcpy(mesh->texcoords2, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model->meshes[i].texcoords2,
                                        testModel.meshes[i].texcoords2, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Normals
        if (meshFlags & 8) {
          size_t size = mesh->vertexCount * 3 * sizeof(float);
          mesh->normals = arena->alloc<float>(size);
          LOG_ASSERT(mesh->normals != nullptr,
                     "Failed to allocate memory for normals: %zu bytes", size);
          memcpy(mesh->normals, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model->meshes[i].normals,
                                        testModel.meshes[i].normals, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Tangents
        if (meshFlags & 16) {
          size_t size = mesh->vertexCount * 4 * sizeof(float);
          mesh->tangents = arena->alloc<float>(size);
          LOG_ASSERT(mesh->tangents != nullptr,
                     "Failed to allocate memory for tangents: %zu bytes", size);
          memcpy(mesh->tangents, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model->meshes[i].tangents,
                                        testModel.meshes[i].tangents, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Colors
        if (meshFlags & 32) {
          size_t size = mesh->vertexCount * 4 * sizeof(unsigned char *);
          mesh->colors = arena->alloc<unsigned char>(size);
          LOG_ASSERT(mesh->colors != nullptr,
                     "Failed to allocate memory for colors: %zu bytes", size);
          memcpy(mesh->colors, data + offset, size);
          LOG_ASSERT(CompareUCharArrays(model->meshes[i].colors,
                                        testModel.meshes[i].colors, size),
                     "testModel and model don't match...");
          offset += size;
        }

        // Animation data
        // Animated vertices
        if (animFlags & 1) {
          size_t size = mesh->vertexCount * 3 * sizeof(float);
          mesh->animVertices = arena->alloc<float>(size);
          LOG_ASSERT(mesh->animVertices != nullptr,
                     "Failed to allocate memory for anim vertices: %zu bytes",
                     size);
          memcpy(mesh->animVertices, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model->meshes[i].animVertices,
                                        testModel.meshes[i].animVertices, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Animated normals
        if (animFlags & 2) {
          size_t size = mesh->vertexCount * 3 * sizeof(float);
          mesh->animNormals = arena->alloc<float>(size);
          LOG_ASSERT(mesh->animNormals != nullptr,
                     "Failed to allocate memory for anim normals: %zu bytes",
                     size);
          memcpy(mesh->animNormals, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model->meshes[i].animNormals,
                                        testModel.meshes[i].animNormals, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Bone IDs
        if (animFlags & 4) {
          size_t size = mesh->vertexCount * 4;
          mesh->boneIds = arena->alloc<unsigned char>(size);
          LOG_ASSERT(mesh->boneIds != nullptr,
                     "Failed to allocate memory for bone IDs: %zu bytes", size);
          memcpy(mesh->boneIds, data + offset, size);
          LOG_ASSERT(CompareUCharArrays(model->meshes[i].boneIds,
                                        testModel.meshes[i].boneIds, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Bone weights
        if (animFlags & 8) {
          size_t size = mesh->vertexCount * 4 * sizeof(float);
          mesh->boneWeights = arena->alloc<float>(size);
          LOG_ASSERT(mesh->boneWeights != nullptr,
                     "Failed to allocate memory for bone weights: %zu bytes",
                     size);
          memcpy(mesh->boneWeights, data + offset, size);
          LOG_ASSERT(CompareFloatArrays(model->meshes[i].boneWeights,
                                        testModel.meshes[i].boneWeights, size),
                     "testModel and model don't match...");
          offset += size;
        }
        // Bone matrices
        if (animFlags & 16 && mesh->boneCount > 0) {
          size_t size = mesh->boneCount * sizeof(Matrix);
          mesh->boneMatrices = arena->alloc<Matrix>(size);
          LOG_ASSERT(mesh->boneMatrices != nullptr,
                     "Failed to allocate memory for bone matrices: %zu bytes",
                     size);
          memcpy(mesh->boneMatrices, data + offset, size);
          LOG_ASSERT(CompareMatrices(model->meshes[i].boneMatrices,
                                     testModel.meshes[i].boneMatrices,
                                     mesh->boneCount),
                     "testModel and model don't match...");
          offset += size;
        }
      }

      // Read indices
      if (mesh->triangleCount > 0 && (meshFlags & 64)) {
        size_t size = mesh->triangleCount * 3 * sizeof(unsigned short);
        mesh->indices = arena->alloc<unsigned short>(size);
        LOG_ASSERT(mesh->indices != nullptr,
                   "Failed to allocate memory for indices: %zu bytes", size);
        memcpy(mesh->indices, data + offset, size);
        LOG_ASSERT(CompareUShortArrays(model->meshes[i].indices,
                                       testModel.meshes[i].indices, size),
                   "testModel and model don't match...");
        offset += size;
      }

      // Read OpenGL identifiers
      UploadMesh(mesh, false);
    }
  }

  // Read materials
  if (globalFlags & 2) {
    int size = model->materialCount * sizeof(Material);
    model->materials = arena->alloc<Material>(size);
    LOG_ASSERT(model->materials != nullptr,
               "Failed to allocate memory for materials: %zu bytes", size);

    for (int i = 0; i < model->materialCount; i++) {
      Material *material = &model->materials[i];

      // Initialize material pointers
      material->shader.locs = nullptr;
      material->maps = nullptr;

      unsigned char matFlags;
      memcpy(&matFlags, data + offset, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      // Read shader ID
      memcpy(&material->shader.id, data + offset, sizeof(unsigned int));
      LOG_ASSERT(model->materials[i].shader.id ==
                     testModel.materials[i].shader.id,
                 "testModel and model don't match...");
      offset += sizeof(unsigned int);

      // Read shader locations
      if (matFlags & 1) {
        size_t size = RL_MAX_SHADER_LOCATIONS * sizeof(int);
        material->shader.locs = arena->alloc<int>(size);
        LOG_ASSERT(material->shader.locs != nullptr,
                   "Failed to allocate memory for shader locations: %zu bytes",
                   size);
        memcpy(material->shader.locs, data + offset, size);
        LOG_ASSERT(CompareShaderLocs(model->materials[i].shader.locs,
                                     testModel.materials[i].shader.locs, size),
                   "testModel and model don't match...");
        offset += size;
      }
      // Read material maps
      if (matFlags & 2) {
        size_t size = MAX_MATERIAL_MAPS * sizeof(MaterialMap);
        material->maps = arena->alloc<MaterialMap>(size);
        LOG_ASSERT(material->maps != nullptr,
                   "Failed to allocate memory for material maps: %zu bytes",
                   size);

        // Read each material map
        for (int j = 0; j < MAX_MATERIAL_MAPS; j++) {
          // Read texture
          memcpy(&material->maps[j].texture, data + offset, sizeof(Texture));
          LOG_ASSERT(CompareTexture(model->materials[i].maps[j].texture,
                                    testModel.materials[i].maps[j].texture),
                     "testModel and model don't match...");
          offset += sizeof(Texture);

          // Read color
          memcpy(&material->maps[j].color, data + offset, sizeof(Color));
          LOG_ASSERT(CompareColor(model->materials[i].maps[j].color,
                                  testModel.materials[i].maps[j].color),
                     "testModel and model don't match...");
          offset += sizeof(Color);

          // Read value
          memcpy(&material->maps[j].value, data + offset, sizeof(float));
          LOG_ASSERT(model->materials[i].maps[j].value ==
                         testModel.materials[i].maps[j].value,
                     "testModel and model don't match...");
          offset += sizeof(float);
        }
      }

      // Read material params (all 4 floats)
      memcpy(&material->params, data + offset, sizeof(float) * 4);
      LOG_ASSERT(CompareFloatArrays(model->materials[i].params,
                                    testModel.materials[i].params,
                                    sizeof(float) * 4),
                 "testModel and model don't match...");
      offset += sizeof(float) * 4;
    }
  }

  // Read mesh material indices
  if (globalFlags & 4) {
    int size = model->meshCount * sizeof(int);
    model->meshMaterial = arena->alloc<int>(size);
    LOG_ASSERT(model->meshMaterial != nullptr,
               "Failed to allocate memory for mesh materials: %zu bytes", size);
    memcpy(model->meshMaterial, data + offset, size);
    LOG_ASSERT(
        CompareMeshMaterials(model->meshMaterial, testModel.meshMaterial, size),
        "testModel and model don't match...");
    offset += size;
  }

  memcpy(&model->boneCount, data + offset, sizeof(int));
  LOG_ASSERT(model->boneCount == testModel.boneCount,
             "testModel and model don't match...");
  offset += sizeof(int);

  if (model->boneCount > 0) {
    // Read bones
    if (globalFlags & 8) {
      int size = model->boneCount * sizeof(BoneInfo);
      model->bones = arena->alloc<BoneInfo>(size);
      LOG_ASSERT(model->bones != nullptr,
                 "Failed to allocate memory for bones: %zu bytes", size);
      memcpy(model->bones, data + offset, size);
      LOG_ASSERT(CompareBones(model->bones, testModel.bones, size),
                 "testModel and model don't match...");
      offset += size;
    }

    // Read bind pose
    if (globalFlags & 16) {
      int size = model->boneCount * sizeof(Transform);
      model->bindPose = arena->alloc<Transform>(size);
      LOG_ASSERT(model->bindPose != nullptr,
                 "Failed to allocate memory for bind pose: %zu bytes", size);

      for (int i = 0; i < model->boneCount; i++) {
        memcpy(&model->bindPose[i].translation, data + offset, sizeof(Vector3));
        LOG_ASSERT(CompareVector3(model->bindPose[i].translation,
                                  testModel.bindPose[i].translation),
                   "testModel and model don't match...");
        offset += sizeof(Vector3);
        memcpy(&model->bindPose[i].rotation, data + offset, sizeof(Vector4));
        LOG_ASSERT(CompareVector4(model->bindPose[i].rotation,
                                  testModel.bindPose[i].rotation),
                   "testModel and model don't match...");
        offset += sizeof(Vector4);
        memcpy(&model->bindPose[i].scale, data + offset, sizeof(Vector3));
        LOG_ASSERT(CompareVector3(model->bindPose[i].scale,
                                  testModel.bindPose[i].scale),
                   "testModel and model don't match...");
        offset += sizeof(Vector3);
      }
    }
  }

  return model;
}

void ExportModelToBinary(const Model &model, const char *filename) {
  if (!filename)
    return;

  FILE *file = fopen(filename, "wb");
  if (!file)
    return;

  size_t offset = 0;

  // Write transform matrix
  fwrite(&model.transform, sizeof(Matrix), 1, file);
  offset += sizeof(Matrix);
  LOG_TRACE("After transform matrix: %zu bytes\n", offset);

  // Write counts
  fwrite(&model.meshCount, sizeof(int), 1, file);
  offset += sizeof(int);
  LOG_TRACE("After mesh count: %zu bytes\n", offset);
  fwrite(&model.materialCount, sizeof(int), 1, file);
  offset += sizeof(int);
  LOG_TRACE("After material count: %zu bytes\n", offset);

  // Write global flags
  unsigned char globalFlags = 0;
  globalFlags |= (model.meshes ? 1 : 0);
  globalFlags |= (model.materials ? 2 : 0);
  globalFlags |= (model.meshMaterial ? 4 : 0);
  globalFlags |= (model.bones ? 8 : 0);
  globalFlags |= (model.bindPose ? 16 : 0);
  fwrite(&globalFlags, sizeof(unsigned char), 1, file);
  offset += sizeof(unsigned char);
  LOG_TRACE("After global flags: %zu bytes\n", offset);

  // Write meshes
  if (model.meshes) {
    for (int i = 0; i < model.meshCount; i++) {
      const Mesh &mesh = model.meshes[i];

      // Write counts
      fwrite(&mesh.vertexCount, sizeof(int), 1, file);
      offset += sizeof(int);
      LOG_TRACE("After mesh %i vertex count: %zu bytes\n", i, offset);
      fwrite(&mesh.triangleCount, sizeof(int), 1, file);
      offset += sizeof(int);
      LOG_TRACE("After mesh %i triangle count: %zu bytes\n", i, offset);
      fwrite(&mesh.boneCount, sizeof(int), 1, file);
      offset += sizeof(int);
      LOG_TRACE("After mesh %i bone count: %zu bytes\n", i, offset);

      // Write mesh flags
      unsigned char meshFlags = 0;
      meshFlags |= (mesh.vertices ? 1 : 0);
      meshFlags |= (mesh.texcoords ? 2 : 0);
      meshFlags |= (mesh.texcoords2 ? 4 : 0);
      meshFlags |= (mesh.normals ? 8 : 0);
      meshFlags |= (mesh.tangents ? 16 : 0);
      meshFlags |= (mesh.colors ? 32 : 0);
      meshFlags |= (mesh.indices ? 64 : 0);
      fwrite(&meshFlags, sizeof(unsigned char), 1, file);
      offset += sizeof(unsigned char);
      LOG_TRACE("After mesh %i mesh flags: %zu bytes\n", i, offset);

      // Write animation flags
      unsigned char animFlags = 0;
      animFlags |= (mesh.animVertices ? 1 : 0);
      animFlags |= (mesh.animNormals ? 2 : 0);
      animFlags |= (mesh.boneIds ? 4 : 0);
      animFlags |= (mesh.boneWeights ? 8 : 0);
      animFlags |= (mesh.boneMatrices ? 16 : 0);
      fwrite(&animFlags, sizeof(unsigned char), 1, file);
      offset += sizeof(unsigned char);
      LOG_TRACE("After mesh %i animation flags: %zu bytes\n", i, offset);

      // Write vertex data
      if (mesh.vertexCount > 0) {
        if (mesh.vertices) {
          fwrite(mesh.vertices, sizeof(float), mesh.vertexCount * 3, file);
          offset += sizeof(float) * mesh.vertexCount * 3;
          LOG_TRACE("After mesh %i vertices: %zu bytes\n", i, offset);
        }
        if (mesh.texcoords) {
          fwrite(mesh.texcoords, sizeof(float), mesh.vertexCount * 2, file);
          offset += sizeof(float) * mesh.vertexCount * 2;
          LOG_TRACE("After mesh %i texcoords: %zu bytes\n", i, offset);
        }
        if (mesh.texcoords2) {
          fwrite(mesh.texcoords2, sizeof(float), mesh.vertexCount * 2, file);
          offset += sizeof(float) * mesh.vertexCount * 2;
          LOG_TRACE("After mesh %i texcoords2: %zu bytes\n", i, offset);
        }
        if (mesh.normals) {
          fwrite(mesh.normals, sizeof(float), mesh.vertexCount * 3, file);
          offset += sizeof(float) * mesh.vertexCount * 3;
          LOG_TRACE("After mesh %i normals: %zu bytes\n", i, offset);
        }
        if (mesh.tangents) {
          fwrite(mesh.tangents, sizeof(float), mesh.vertexCount * 4, file);
          offset += sizeof(float) * mesh.vertexCount * 4;
          LOG_TRACE("After mesh %i tangents: %zu bytes\n", i, offset);
        }
        if (mesh.colors) {
          fwrite(mesh.colors, sizeof(unsigned char), mesh.vertexCount * 4,
                 file);
          offset += sizeof(unsigned char) * mesh.vertexCount * 4;
          LOG_TRACE("After mesh %i colors: %zu bytes\n", i, offset);
        }

        // Write animation data
        if (mesh.animVertices) {
          fwrite(mesh.animVertices, sizeof(float), mesh.vertexCount * 3, file);
          offset += sizeof(float) * mesh.vertexCount * 3;
          LOG_TRACE("After mesh %i anim vertices: %zu bytes\n", i, offset);
        }
        if (mesh.animNormals) {
          fwrite(mesh.animNormals, sizeof(float), mesh.vertexCount * 3, file);
          offset += sizeof(float) * mesh.vertexCount * 3;
          LOG_TRACE("After mesh %i anim normals: %zu bytes\n", i, offset);
        }
        if (mesh.boneIds) {
          fwrite(mesh.boneIds, sizeof(unsigned char), mesh.vertexCount * 4,
                 file);
          offset += sizeof(unsigned char) * mesh.vertexCount * 4;
          LOG_TRACE("After mesh %i bone IDs: %zu bytes\n", i, offset);
        }
        if (mesh.boneWeights) {
          fwrite(mesh.boneWeights, sizeof(float), mesh.vertexCount * 4, file);
          offset += sizeof(float) * mesh.vertexCount * 4;
          LOG_TRACE("After mesh %i bone weights: %zu bytes\n", i, offset);
        }
        if (mesh.boneMatrices && mesh.boneCount > 0) {
          fwrite(mesh.boneMatrices, sizeof(Matrix), mesh.boneCount, file);
          offset += sizeof(Matrix) * mesh.boneCount;
          LOG_TRACE("After mesh %i boneMatrices: %zu bytes\n", i, offset);
        }
      }

      // Write indices
      if (mesh.triangleCount > 0 && mesh.indices) {
        fwrite(mesh.indices, sizeof(unsigned short), mesh.triangleCount * 3,
               file);
        offset += sizeof(unsigned short) * mesh.triangleCount * 3;
        LOG_TRACE("After mesh %i indices: %zu bytes\n", i, offset);
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
      fwrite(&matFlags, sizeof(unsigned char), 1, file);
      offset += sizeof(unsigned short);
      LOG_TRACE("After material %i material flags: %zu bytes\n", i, offset);

      // Write shader
      fwrite(&material.shader.id, sizeof(unsigned int), 1, file);
      offset += sizeof(unsigned int);
      LOG_TRACE("After material %i shader id: %zu bytes\n", i, offset);

      if (material.shader.locs) {
        fwrite(material.shader.locs, sizeof(int), RL_MAX_SHADER_LOCATIONS,
               file);
        offset += sizeof(int) * RL_MAX_SHADER_LOCATIONS;
        LOG_TRACE("After material %i shader locs: %zu bytes\n", i, offset);
      }

      // Write material maps
      if (material.maps) {
        for (int j = 0; j < MAX_MATERIAL_MAPS; j++) {
          const MaterialMap &map = material.maps[j];
          fwrite(&map.texture, sizeof(Texture), 1, file);
          offset += sizeof(Texture);
          LOG_TRACE("After material %i map %i texture: %zu bytes\n", i, j,
                    offset);
          fwrite(&map.color, sizeof(Color), 1, file);
          offset += sizeof(Color);
          LOG_TRACE("After material %i map %i color: %zu bytes\n", i, j,
                    offset);
          fwrite(&map.value, sizeof(float), 1, file);
          offset += sizeof(float);
          LOG_TRACE("After material %i map %i value: %zu bytes\n", i, j,
                    offset);
        }
      }

      // Write material parameters
      fwrite(material.params, sizeof(float), 4, file);
      offset += sizeof(float) * 4;
      LOG_TRACE("After material %i params: %zu bytes\n", i, offset);
    }
  }

  // Write mesh material indices
  if (model.meshMaterial) {
    fwrite(model.meshMaterial, sizeof(int), model.meshCount, file);
    offset += sizeof(int) * model.meshCount;
    LOG_TRACE("After mesh material: %zu bytes\n", offset);
  }

  // BoneCount
  fwrite(&model.boneCount, sizeof(int), 1, file);
  offset += sizeof(int);
  LOG_TRACE("After bone count: %zu bytes\n", offset);

  if (model.boneCount > 0) {
    // BoneInfo
    if (model.bones) {
      fwrite(model.bones, sizeof(BoneInfo), model.boneCount, file);
      offset += sizeof(BoneInfo) * model.boneCount;
      LOG_TRACE("After bone info: %zu bytes\n", offset);
    }

    // bindPose
    if (model.bindPose) {
      for (int i = 0; i < model.boneCount; i++) {
        fwrite(&model.bindPose[i].translation, sizeof(Vector3), 1, file);
        offset += sizeof(Vector3);
        LOG_TRACE("After bind pose %i translation: %zu bytes\n", i, offset);
        fwrite(&model.bindPose[i].rotation, sizeof(Vector4), 1, file);
        offset += sizeof(Vector4);
        LOG_TRACE("After bind pose %i rotation: %zu bytes\n", i, offset);
        fwrite(&model.bindPose[i].scale, sizeof(Vector3), 1, file);
        offset += sizeof(Vector3);
        LOG_TRACE("After bind pose %i scale: %zu bytes\n", i, offset);
      }
    }
  }

  fclose(file);
}

char **listFiles(const char* path, int &count, Arena* arena) {
  DIR *dir;
  struct dirent *entry;
  char **files = nullptr;
  count = 0;
  int capacity = 10;

  files = arena->alloc<char*>(capacity * sizeof(char *));
  if (!files)
    return nullptr;

  dir = opendir(path);
  if (!dir) {
    return nullptr;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) { // Regular file
      if (strstr(entry->d_name, ".glb")) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (count >= capacity) {
          capacity *= 2;
          char **temp = (char **)realloc(files, capacity * sizeof(char *));
          if (!temp) {
            // Handle error
            break;
          }
          files = temp;
        }

        files[count] = strdup(fullpath);
        count++;
      }
    } else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 &&
               strcmp(entry->d_name, "..") != 0) {
      char subpath[1024];
      snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);

      int subcount = 0;
      char **subfiles = listFiles(subpath, subcount, arena);

      if (subfiles) {
        if (count + subcount >= capacity) {
          capacity = count + subcount;
          char **temp = (char **)realloc(files, capacity * sizeof(char *));
          if (!temp) {
            // Handle error
            break;
          }
          files = temp;
        }

        for (int i = 0; i < subcount; i++) {
          files[count + i] = subfiles[i];
        }
        count += subcount;
      }
    }
  }

  closedir(dir);
  return files;
}

int main(int argc, char *argv[]) {
  InitWindow(800, 450, "prep models");

  Arena* arena = new Arena(1024 * 1024);
  MapCT<char*, Model, 100> modelMap = *arena->create_map_ct<char*, Model, 100>();

  int count = 0;
  char **models = listFiles("resources/models", count, arena);

  for (int i = 0; i < count; i++) {
    const char *in = models[i];
    const char *filename = strrchr(in, '/');
    filename = filename ? filename + 1 : in;

    char filenameBin[256];
    snprintf(filenameBin, sizeof(filenameBin), "%.*s.bin",
             (int)(strlen(filename) - 4), filename);
    char out[256];
    snprintf(out, sizeof(out), "./resources/models/%s", filenameBin);

    LOG_TRACE("%s -> %s", in, out);
    Model model = LoadModel(in);
    ExportModelToBinary(model, out);
    modelMap[filenameBin] = model;
  }

  system("./libs/rrespacker/rrespacker -o resources.rres --rrp resources.rrp");

  rresCentralDir dir = rresLoadCentralDirectory("resources.rres");
  for (auto &[path, testModel] : modelMap) {
    int idModel = rresGetResourceId(dir, path);
    rresResourceChunk chunkModel =
        rresLoadResourceChunk("resources.rres", idModel);
    Model* modelTest = LoadModelFromChunk(chunkModel, testModel, arena);
    UnloadModel(testModel);
    rresUnloadResourceChunk(chunkModel);
  }

  delete arena;
  rresUnloadCentralDirectory(dir);

  return 0;
}
