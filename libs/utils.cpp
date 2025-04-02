#include "utils.h"
#include <cctype>
#include <cstddef>
#include <cstdio>

// NOTE: File I/O
long long get_timestamp(const char* filePath) noexcept {
  struct stat file_stat = {};
  stat(filePath, &file_stat);
  return file_stat.st_mtime;
}

bool file_exists(const char* filePath) noexcept {
  LOG_ASSERT(filePath, "No filePath supplied!");

  auto file = fopen(filePath, "rb");
  if (!file) {
    return false;
  }
  fclose(file);

  return true;
}

size_t get_file_size(const char* filePath) noexcept {
  LOG_ASSERT(filePath, "No filePath supplied!");

  size_t fileSize = 0;
  auto file = fopen(filePath, "rb");
  if (!file) {
    LOG_ERROR("Failed opening File: %s", filePath);
    return 0;
  }

  fseek(file, 0, SEEK_END);
  fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  fclose(file);

  return fileSize;
}

char* read_file(const char* filePath, Arena& arena) noexcept {
  LOG_ASSERT(filePath, "No filePath supplied!");
  size_t fileSize = get_file_size(filePath);

  char* buffer = arena.alloc_raw<char>(fileSize + 1);
  auto file = fopen(filePath, "rb");
  fread(buffer, sizeof(char), fileSize, file);
  fclose(file);

  return buffer; 
}

void write_file(const char* filePath, const char* buffer, size_t size) noexcept {
  LOG_ASSERT(filePath, "No filePath supplied!");
  LOG_ASSERT(buffer, "No buffer supplied!");
  auto file = fopen(filePath, "wb");
  if (!file) {
    LOG_ERROR("Failed opening File: %s", filePath);
    return;
  }

  fwrite(buffer, sizeof(char), size, file);
  fclose(file);
}

bool copy_file(const char* filePath, const char* outputPath, Arena& arena) noexcept {
  char* data = read_file(filePath, arena);

  auto outputFile = fopen(outputPath, "wb");
  if (!outputFile) {
    LOG_ERROR("Failed opening File: %s", outputPath);
    return false;
  }

  size_t fileSize = get_file_size(filePath);
  int result = fwrite(data, sizeof(char), fileSize, outputFile);
  if (!result) {
    LOG_ERROR("Failed opening File: %s", outputPath);
    return false;
  }

  fclose(outputFile);
  return true;
}

// Wrapper around remove() for consistent naming
void remove_file(const char* filePath) noexcept {
  remove(filePath);
}

// NOTE: Testing
bool CompareFloat(float a, float b, float epsilon) noexcept {
  return fabs(a - b) <= epsilon;
}

bool CompareIntArrays(const int* a, const int* b, size_t size) noexcept {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;
  return memcmp(a, b, size) == 0;
}

bool CompareFloatArrays(const float *a, const float *b, size_t size) noexcept {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;
  return memcmp(a, b, size) == 0;
}

bool CompareUCharArrays(const unsigned char* a, const unsigned char* b, size_t size) noexcept {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;
  return memcmp(a, b, size) == 0;
}

bool CompareUIntArrays(const unsigned int* a, const unsigned int* b, size_t size) noexcept {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;
  return memcmp(a, b, size) == 0;
}

bool CompareUShortArrays(const unsigned short* a, const unsigned short* b, size_t size) noexcept {
  if ((!a && b) || (a && !b))
    return false;
  if (!a && !b)
    return true;
  return memcmp(a, b, size) == 0;
}
