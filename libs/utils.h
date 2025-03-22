#pragma once

#include <cstddef>
#include <stdio.h>
#include <stdlib.h> // This is to get malloc
#include <string.h> // This is to get memset
#include <sys/stat.h> // Used to get the edit timestamp of files
#include <cmath>
#include <cstring>

// NOTE: Cross platform stuffs
#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#define EXPORT_FN __declspec(dllexport)
#elif __linux__
#define DEBUG_BREAK() __builtin_trap()
#define EXPORT_FN extern "C"
#elif __APPLE__
#define DEBUG_BREAK() __builtin_trap()
#define EXPORT_FN
#endif

// NOTE: Logging
enum TextColor
{  
  TEXT_COLOR_BLACK,
  TEXT_COLOR_RED,
  TEXT_COLOR_GREEN,
  TEXT_COLOR_YELLOW,
  TEXT_COLOR_BLUE,
  TEXT_COLOR_MAGENTA,
  TEXT_COLOR_CYAN,
  TEXT_COLOR_WHITE,
  TEXT_COLOR_BRIGHT_BLACK,
  TEXT_COLOR_BRIGHT_RED,
  TEXT_COLOR_BRIGHT_GREEN,
  TEXT_COLOR_BRIGHT_YELLOW,
  TEXT_COLOR_BRIGHT_BLUE,
  TEXT_COLOR_BRIGHT_MAGENTA,
  TEXT_COLOR_BRIGHT_CYAN,
  TEXT_COLOR_BRIGHT_WHITE,
  TEXT_COLOR_COUNT
};

template <typename ...Args>
void _log(char* prefix, char* msg, TextColor textColor, Args... args)
{
  const static char* TextColorTable[TEXT_COLOR_COUNT] = 
  {    
    "\x1b[30m", // TEXT_COLOR_BLACK
    "\x1b[31m", // TEXT_COLOR_RED
    "\x1b[32m", // TEXT_COLOR_GREEN
    "\x1b[33m", // TEXT_COLOR_YELLOW
    "\x1b[34m", // TEXT_COLOR_BLUE
    "\x1b[35m", // TEXT_COLOR_MAGENTA
    "\x1b[36m", // TEXT_COLOR_CYAN
    "\x1b[37m", // TEXT_COLOR_WHITE
    "\x1b[90m", // TEXT_COLOR_BRIGHT_BLACK
    "\x1b[91m", // TEXT_COLOR_BRIGHT_RED
    "\x1b[92m", // TEXT_COLOR_BRIGHT_GREEN
    "\x1b[93m", // TEXT_COLOR_BRIGHT_YELLOW
    "\x1b[94m", // TEXT_COLOR_BRIGHT_BLUE
    "\x1b[95m", // TEXT_COLOR_BRIGHT_MAGENTA
    "\x1b[96m", // TEXT_COLOR_BRIGHT_CYAN
    "\x1b[97m", // TEXT_COLOR_BRIGHT_WHITE
  };

  char formatBuffer[8192] = {};
  sprintf(formatBuffer, "%s %s %s \033[0m", TextColorTable[textColor], prefix, msg);

  char textBuffer[8912] = {};
  sprintf(textBuffer, formatBuffer, args...);

  puts(textBuffer);
}

#define LOG_TRACE(msg, ...) _log("TRACE: ", msg, TEXT_COLOR_GREEN, ##__VA_ARGS__);
#define LOG_WARN(msg, ...) _log("WARN: ", msg, TEXT_COLOR_YELLOW, ##__VA_ARGS__);
#define LOG_ERROR(msg, ...) _log("ERROR: ", msg, TEXT_COLOR_RED, ##__VA_ARGS__);

#define LOG_ASSERT(x, msg, ...)      \
{                                    \
  if(!(x))                           \
  {                                  \
    LOG_ERROR(msg, ##__VA_ARGS__);   \
    DEBUG_BREAK();                   \
    LOG_ERROR("Assertion HIT!")      \
  }                                  \
}

// NOTE: Array

template <typename T>
struct ArrayIterator {
    T* ptr;
    T* end;

    ArrayIterator(const ArrayIterator&) = delete;
    ArrayIterator& operator=(const ArrayIterator&) = delete;
    ArrayIterator(ArrayIterator&& other) = delete;
    ArrayIterator& operator=(ArrayIterator&& other) = delete;

    ArrayIterator(T* start, T* end) 
        : ptr(start), end(end) {}

    ArrayIterator& operator++() {
        if (ptr != end) {
            ++ptr;
        }
        return *this;
    }

    bool operator!=(const ArrayIterator& other) const { return ptr != other.ptr; }
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
};

template<typename T, int N>
struct ArrayCT {
  static constexpr int maxElements = N;
  int count = 0;
  T elements[maxElements];

  ArrayCT(const ArrayCT&) = delete;
  ArrayCT& operator=(const ArrayCT&) = delete;
  ArrayCT(ArrayCT&& other) = delete;
  ArrayCT& operator=(ArrayCT&& other) = delete;

  T& get(int idx) {
    LOG_ASSERT(idx >= 0 && idx < count, "Index out of bounds!");
    return elements[idx];
  }

  T& operator[](int idx) {
    return this->get(idx);
  }

  int add(T element) {
    LOG_ASSERT(count + 1 <= maxElements, "Array Full!");
    elements[count] = element;
    return count++;
  }

  int add(const T* elements_list, int num_elements) {
    LOG_ASSERT(count + num_elements <= maxElements, "Would overflow array!");
    for (int i = 0; i < num_elements; ++i) {
      add(elements_list[i]);
    }
    return count;
  }

  void remove(int idx) { //O(1) but doesn't keep order (swap to last index & decrement)
    LOG_ASSERT(idx < count, "idx out of bounds!");
    elements[idx] = elements[--count];
  }

  void clear() noexcept {
    count = 0;
  }

  bool is_full() const noexcept {
    return count == N;
  }

  bool empty() const noexcept {
      return count == 0;
  }

  size_t size() const noexcept {
      return count;
  }

  using Iterator = ArrayIterator<T>;
  Iterator begin() { return Iterator(elements, elements + count); }
  Iterator end() { return Iterator(elements + count, elements + count); }
};

template<typename T>
struct ArrayRT {
  int capacity;  // runtime capacity, set when allocated
  int count = 0; // number of elements added so far
  T elements[1]; // Flexible array member (allocate extra space)

  ArrayRT(const ArrayRT&) = delete;
  ArrayRT& operator=(const ArrayRT&) = delete;
  ArrayRT(ArrayRT&& other) = delete;
  ArrayRT& operator=(ArrayRT&& other) = delete;

  T& get(int idx) {
    LOG_ASSERT(idx >= 0 && idx < count, "Index out of bounds!");
    return elements[idx];
  }

  T& operator[](int idx) {
    return this->get(idx);
  }

  int add(T element) {
    LOG_ASSERT(count < capacity, "Array Full!");
    elements[count] = element;
    return count++;
  }

  int add(const T* elements_list, int num_elements) {
    LOG_ASSERT(count + num_elements <= capacity, "Would overflow array!");
    for (int i = 0; i < num_elements; ++i)
      add(elements_list[i]);
    return count;
  }

  void remove(int idx) { // O(1): swap with last and decrement
    LOG_ASSERT(idx < count, "Index out of bounds!");
    elements[idx] = elements[--count];
  }

  void clear() noexcept {
    count = 0;
  }

  bool is_full() const noexcept{
    return count == capacity;
  }

  bool empty() const noexcept {
      return count == 0;
  }

  size_t size() const noexcept {
      return count;
  }

  using Iterator = ArrayIterator<T>;
  Iterator begin() { return Iterator(elements, elements + count); }
  Iterator end() { return Iterator(elements + count, elements + count); }
};

//NOTE: Map

template <typename KeyType, typename ValueType>
struct Entry {
  KeyType key;
  ValueType value;
};

template <typename KeyType, typename ValueType>
struct MapIterator {
    Entry<KeyType, ValueType>* ptr;
    Entry<KeyType, ValueType>* end;

    MapIterator(const MapIterator&) = delete;
    MapIterator& operator=(const MapIterator&) = delete;
    MapIterator(MapIterator&& other) = delete;
    MapIterator& operator=(MapIterator&& other) = delete;

    MapIterator(Entry<KeyType, ValueType>* start, Entry<KeyType, ValueType>* end) 
        : ptr(start), end(end) {}

    MapIterator& operator++() {
        if (ptr != end) {
            ++ptr;
        }
        return *this;
    }

    bool operator!=(const MapIterator& other) const { return ptr != other.ptr; }
    Entry<KeyType, ValueType>& operator*() const { return *ptr; }
    Entry<KeyType, ValueType>* operator->() const { return ptr; }
};

template <typename KeyType, typename ValueType, int Size>
struct MapCT {
  ArrayCT<Entry<KeyType, ValueType>, Size> entries;

  MapCT(const MapCT&) = delete;
  MapCT& operator=(const MapCT&) = delete;
  MapCT(MapCT&& other) = delete;
  MapCT& operator=(MapCT&& other) = delete;

  // Linear search to find an entry by key
  int find(KeyType key) const {
    for (int i = 0; i < entries.count; ++i) {
      if (entries.elements[i].key == key) {
        return i; // Return the index of the found entry
      }
    }
    return -1; // Return -1 if the key is not found
  }

  ValueType& get(KeyType key) {
      int idx = find(key);
      if (idx == -1) {
          idx = entries.add(Entry<KeyType, ValueType>{key, ValueType{}});
      }
      return entries[idx].value;
  }

  ValueType& operator[](KeyType key) {
      return this->get(key);
  }

  bool empty() const noexcept {
      return entries.empty();
  }

  size_t size() const noexcept {
      return entries.size();
  }

  using Iterator = MapIterator<KeyType, ValueType>;
  Iterator begin() { return Iterator(entries.elements, entries.elements + entries.count); }
  Iterator end() { return Iterator(entries.elements + entries.count, entries.elements + entries.count); }
};

template <typename KeyType, typename ValueType>
struct MapRT {
  ArrayRT<Entry<KeyType, ValueType>>* entries; // set when allocated

  MapRT(const MapRT&) = delete;
  MapRT& operator=(const MapRT&) = delete;
  MapRT(MapRT&& other) = delete;
  MapRT& operator=(MapRT&& other) = delete;

  // Linear search to find an entry by key
  int find(KeyType key) const {
    LOG_ASSERT(entries != nullptr, "Null entries pointer!");
    for (int i = 0; i < entries->count; ++i) {
      if (entries->elements[i].key == key) {
        return i; // Return the index of the found entry
      }
    }
    return -1; // Return -1 if the key is not found
  }

  ValueType& get(KeyType key) {
      int idx = find(key);
      if (idx == -1) {
          idx = entries->add(Entry<KeyType, ValueType>{key, ValueType{}});
      }
      return (*entries)[idx].value;
  }

  ValueType& operator[](KeyType key) {
      return this->get(key);
  }

  bool empty() const noexcept {
      return entries->empty();
  }

  size_t size() const noexcept {
      return entries->size();
  }

  using Iterator = MapIterator<KeyType, ValueType>;
  Iterator begin() { return Iterator(entries->elements, entries->elements + entries->count); }
  Iterator end() { return Iterator(entries->elements + entries->count, entries->elements + entries->count); }
};

// NOTE: Arena
struct Arena {
  size_t capacity;
  size_t used;
  char* memory;

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;
  Arena(Arena&& other) = delete;
  Arena& operator=(Arena&& other) = delete;

  Arena(size_t size) {
    memory = (char*)malloc(size);
    if (!memory) LOG_ASSERT(false, "Failed to allocate memory!");
    memset(memory, 0, size); // Sets the memory to 0
    capacity = size;
    used = 0;
  }

  char& operator[](size_t idx) {
    LOG_ASSERT(idx < used, "Index out of bounds!");
    return memory[idx];
  }

  template<typename T>
  T* alloc() {
    size_t size = sizeof(T);
    size_t aligned_size = (size + 7) & ~7;  // 8-byte alignment
    if (used + aligned_size > capacity) LOG_ASSERT(false, "Arena is full");
    memset(memory + used, 0, aligned_size); // Sets the memory to 0
    T* result = (T*)(memory + used);
    used += aligned_size;
    return result;
  }

  template<typename T>
  T* alloc(size_t size) {
    size_t aligned_size = (size + 7) & ~7;  // 8-byte alignment
    if (used + aligned_size > capacity) LOG_ASSERT(false, "Arena is full");
    memset(memory + used, 0, aligned_size); // Sets the memory to 0
    T* result = (T*)(memory + used);
    used += aligned_size;
    return result;
  }

  template<typename T>
  T* alloc_count(size_t count) {
    size_t total_size = sizeof(T) * count;
    size_t aligned_size = (total_size + 7) & ~7;  // 8-byte alignment
    if (used + aligned_size > capacity) LOG_ASSERT(false, "Arena is full");
    T* result = reinterpret_cast<T*>(memory + used);
    used += aligned_size;
    return result;
  }

  template <typename E, typename M> 
  E* fetch(char* key) {
    M* map = reinterpret_cast<M*>(this->memory);
    void* ptr = (*map)[key];
    LOG_ASSERT(ptr, "Arena index map doesn't contain key: %s", key);
    E* element = reinterpret_cast<E*>(ptr);
    return element;
  }

  template<typename T>
  ArrayRT<T>* create_array_rt(size_t capacity) {
    size_t total_size = sizeof(ArrayRT<T>) + sizeof(T) * (capacity - 1);
    ArrayRT<T>* arr = this->alloc<ArrayRT<T>>(total_size);
    arr->capacity = capacity;
    return arr;
  }

  template<typename T, int N>
  ArrayCT<T, N>* create_array_ct() {
    ArrayCT<T, N>* arr = this->alloc<ArrayCT<T, N>>();
    return arr;
  }

  template<typename KeyType, typename ValueType>
  MapRT<KeyType, ValueType>* create_map_rt(size_t capacity) {
    MapRT<KeyType, ValueType>* map = this->alloc<MapRT<KeyType, ValueType>>(sizeof(MapRT<KeyType, ValueType>));
    ArrayRT<Entry<KeyType, ValueType>>* entries = this->create_array_rt<Entry<KeyType, ValueType>>(capacity);
    map->entries = entries;
    return map;
  }

  template<typename KeyType, typename ValueType, int N>
  MapCT<KeyType, ValueType, N>* create_map_ct() {
    MapCT<KeyType, ValueType, N>* map = this->alloc<MapCT<KeyType, ValueType, N>>();
    return map;
  }

  void clear() noexcept {
    used = 0;
  }

  size_t size() const noexcept {
      return used;
  }

  size_t available() const noexcept {
      return capacity - used;
  }

  bool is_empty() const noexcept {
      return used == 0;
  }

  ~Arena() {
    free(memory);
  }
};

// NOTE: Size defs
#define KB(x) ((x) * 1024ULL)
#define MB(x) ((x) * 1024ULL * 1024ULL)
#define GB(x) ((x) * 1024ULL * 1024ULL * 1024ULL)

// NOTE: File I/O
long long get_timestamp(const char* file);
bool file_exists(const char* filePath);
size_t get_file_size(const char* filePath);
char* read_file(const char* filePath, Arena* arena);
void write_file(const char* filePath, char* buffer, size_t size);
bool copy_file(const char* fileName, const char* outputName, Arena* arena);
void remove_file(const char* fileName);

//NOTE: Testing
bool CompareFloat(float a, float b, float epsilon = 0.0001f);
bool CompareIntArrays(const int *a, const int *b, size_t size);
bool CompareFloatArrays(const float *a, const float *b, size_t size);
bool CompareUCharArrays(const unsigned char *a, const unsigned char *b, size_t size);
bool CompareUIntArrays(const unsigned int *a, const unsigned int *b, size_t size);
bool CompareUShortArrays(const unsigned short *a, const unsigned short *b, size_t size);
