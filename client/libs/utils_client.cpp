#include "utils_client.h"

// NOTE: Load model from chunk for use with rres
Model& LoadModelFromChunk(const rresResourceChunk &chunk, Arena& arena) {
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
      offset += sizeof(int);
      memcpy(&mesh.triangleCount, data + offset, sizeof(int));
      offset += sizeof(int);
      memcpy(&mesh.boneCount, data + offset, sizeof(int));
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
          offset += size;
        }
        // Normals
        if (meshFlags & 8) {
          size_t size = mesh.vertexCount * 3 * sizeof(float);
          mesh.normals = arena.alloc_raw<float>(size);
          LOG_ASSERT(mesh.normals != nullptr,
                     "Failed to allocate memory for normals: %zu bytes", size);
          memcpy(mesh.normals, data + offset, size);
          offset += size;
        }
        // Tangents
        if (meshFlags & 16) {
          size_t size = mesh.vertexCount * 4 * sizeof(float);
          mesh.tangents = arena.alloc_raw<float>(size);
          LOG_ASSERT(mesh.tangents != nullptr,
                     "Failed to allocate memory for tangents: %zu bytes", size);
          memcpy(mesh.tangents, data + offset, size);
          offset += size;
        }
        // Colors
        if (meshFlags & 32) {
          size_t size = mesh.vertexCount * 4 * sizeof(unsigned char *);
          mesh.colors = arena.alloc_raw<unsigned char>(size);
          LOG_ASSERT(mesh.colors != nullptr,
                     "Failed to allocate memory for colors: %zu bytes", size);
          memcpy(mesh.colors, data + offset, size);
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
          offset += size;
        }
        // Bone IDs
        if (animFlags & 4) {
          size_t size = mesh.vertexCount * 4;
          mesh.boneIds = arena.alloc_raw<unsigned char>(size);
          LOG_ASSERT(mesh.boneIds != nullptr,
                     "Failed to allocate memory for bone IDs: %zu bytes", size);
          memcpy(mesh.boneIds, data + offset, size);
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
      offset += sizeof(unsigned int);

      // Read shader locations
      if (matFlags & 1) {
        size_t size = RL_MAX_SHADER_LOCATIONS * sizeof(int);
        material.shader.locs = arena.alloc_raw<int>(size);
        LOG_ASSERT(material.shader.locs != nullptr,
                   "Failed to allocate memory for shader locations: %zu bytes",
                   size);
        memcpy(material.shader.locs, data + offset, size);
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
          offset += sizeof(Texture);

          // Read color
          memcpy(&material.maps[j].color, data + offset, sizeof(Color));
          offset += sizeof(Color);

          // Read value
          memcpy(&material.maps[j].value, data + offset, sizeof(float));
          offset += sizeof(float);
        }
      }

      // Read material params (all 4 floats)
      memcpy(&material.params, data + offset, sizeof(float) * 4);
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
    offset += size;
  }

  memcpy(&model.boneCount, data + offset, sizeof(int));
  offset += sizeof(int);

  if (model.boneCount > 0) {
    // Read bones
    if (globalFlags & 8) {
      int size = model.boneCount * sizeof(BoneInfo);
      model.bones = arena.alloc_raw<BoneInfo>(size);
      LOG_ASSERT(model.bones != nullptr,
                 "Failed to allocate memory for bones: %zu bytes", size);
      memcpy(model.bones, data + offset, size);
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

// NOTE: Comparisons
bool CompareVector3(const Vector3 &a, const Vector3 &b, float epsilon) {
  return CompareFloat(a.x, b.x, epsilon) && CompareFloat(a.y, b.y, epsilon) &&
         CompareFloat(a.z, b.z, epsilon);
}

bool CompareVector4(const Vector4 &a, const Vector4 &b, float epsilon) {
  return CompareFloat(a.x, b.x, epsilon) && CompareFloat(a.y, b.y, epsilon) &&
         CompareFloat(a.z, b.z, epsilon) && CompareFloat(a.w, b.w, epsilon);
}

bool CompareBones(const BoneInfo *a, const BoneInfo *b, size_t size) {
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
