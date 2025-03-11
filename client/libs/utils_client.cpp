#include "raylib.h"
#include <cmath>
#include <cstring>

//NOTE: Comparisons
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
