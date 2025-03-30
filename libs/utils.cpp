#include "utils.h"
#include <cctype>
#include <cstddef>
#include <cstdio>

// NOTE: Map
bool is_string_key(const void* key, size_t size) noexcept {
    // Handle char* and const char*
    if (size == sizeof(char*)) {
        const char* str = *(const char**)key;
        return str != nullptr && (str[0] == '\0' || isprint(str[0]));
    }
    
    // Handle char arrays, but verify it's actually a string
    const char* test = static_cast<const char*>(key);
    // Check if size is reasonable for a string and first char is valid
    return size > 1 && size < 1024 && // reasonable size limit
           (test[0] == '\0' || isprint(test[0])) &&
           // Verify null termination within the size
           memchr(test, '\0', size) != nullptr;
}

bool compare_keys(const void* a, const void* b, size_t size) noexcept {
    if (is_string_key(a, size)) {
        if (size == sizeof(char*)) {
            const char* str_a = *(const char**)a;
            const char* str_b = *(const char**)b;
            return strcmp(str_a, str_b) == 0;
        } else {
            const char* str_a = static_cast<const char*>(a);
            const char* str_b = static_cast<const char*>(b);
            return strcmp(str_a, str_b) == 0;
        }
    }
    
    switch(size) {
        case sizeof(uint32_t):
            return *(const uint32_t*)a == *(const uint32_t*)b;
        case sizeof(uint64_t):
            return *(const uint64_t*)a == *(const uint64_t*)b;
        default:
            return memcmp(a, b, size) == 0;
    }
}

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
