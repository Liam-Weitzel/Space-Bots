#include "utils.h"

// NOTE: File I/O
long long get_timestamp(const char* file) {
  struct stat file_stat = {};
  stat(file, &file_stat);
  return file_stat.st_mtime;
}

bool file_exists(const char* filePath) {
  LOG_ASSERT(filePath, "No filePath supplied!");

  auto file = fopen(filePath, "rb");
  if(!file) {
    return false;
  }
  fclose(file);

  return true;
}

long get_file_size(const char* filePath) {
  LOG_ASSERT(filePath, "No filePath supplied!");

  long fileSize = 0;
  auto file = fopen(filePath, "rb");
  if(!file) {
    LOG_ERROR("Failed opening File: %s", filePath);
    return 0;
  }

  fseek(file, 0, SEEK_END);
  fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  fclose(file);

  return fileSize;
}

/*
* Reads a file into a supplied buffer. We manage our own
* memory and therefore want more control over where it 
* is allocated
*/
char* read_file(const char* filePath, int* fileSize, char* buffer) {
  LOG_ASSERT(filePath, "No filePath supplied!");
  LOG_ASSERT(fileSize, "No fileSize supplied!");
  LOG_ASSERT(buffer, "No buffer supplied!");

  *fileSize = 0;
  auto file = fopen(filePath, "rb");
  if(!file) {
    LOG_ERROR("Failed opening File: %s", filePath);
    return nullptr;
  }

  fseek(file, 0, SEEK_END);
  *fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  memset(buffer, 0, *fileSize + 1);
  fread(buffer, sizeof(char), *fileSize, file);

  fclose(file);

  return buffer;
}

char* read_file(const char* filePath, int* fileSize, Arena* arena) {
  char* file = nullptr;
  size_t fileSize2 = get_file_size(filePath);

  if(fileSize2) {
    char* buffer = arena->alloc<char>(fileSize2 + 1);

    file = read_file(filePath, fileSize, buffer);
  }

  return file; 
}

void write_file(const char* filePath, char* buffer, int size) {
  LOG_ASSERT(filePath, "No filePath supplied!");
  LOG_ASSERT(buffer, "No buffer supplied!");
  auto file = fopen(filePath, "wb");
  if(!file) {
    LOG_ERROR("Failed opening File: %s", filePath);
    return;
  }

  fwrite(buffer, sizeof(char), size, file);
  fclose(file);
}

bool copy_file(const char* fileName, const char* outputName, char* buffer) {
  int fileSize = 0;
  char* data = read_file(fileName, &fileSize, buffer);

  auto outputFile = fopen(outputName, "wb");
  if(!outputFile) {
    LOG_ERROR("Failed opening File: %s", outputName);
    return false;
  }

  int result = fwrite(data, sizeof(char), fileSize, outputFile);
  if(!result) {
    LOG_ERROR("Failed opening File: %s", outputName);
    return false;
  }
  
  fclose(outputFile);

  return true;
}

bool copy_file(const char* fileName, const char* outputName, Arena* arena) {
  char* file = 0;
  size_t fileSize2 = get_file_size(fileName);

  if(fileSize2) {
    char* buffer = arena->alloc<char>(fileSize2 + 1);
    return copy_file(fileName, outputName, buffer);
  }

  return false;
}

//NOTE: Testing
bool CompareFloat(float a, float b, float epsilon) {
  return fabs(a - b) <= epsilon;
}

bool CompareIntArrays(const int *a, const int *b, size_t size) {
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
