#pragma once

#include "raylib.h"
#include "rlgl.h"
#include "utils.h"
#include "rres.h"
#include <cmath>
#include <cstring>

#define MAX_MATERIAL_MAPS 12
#define RL_MAX_SHADER_LOCATIONS 32

// NOTE: Load model from chunk for use with rres
Model *LoadModelFromChunk(const rresResourceChunk &chunk, Arena* arena);

// NOTE: Comparisons
bool CompareVector3(const Vector3 &a, const Vector3 &b, float epsilon = 0.0001f);
bool CompareVector4(const Vector4 &a, const Vector4 &b, float epsilon = 0.0001f);
bool CompareBones(const BoneInfo *a, const BoneInfo *b, size_t size);
bool CompareMatrices(const Matrix *a, const Matrix *b, size_t count);
bool CompareColor(const Color &a, const Color &b);
bool CompareTexture(const Texture &a, const Texture &b);
