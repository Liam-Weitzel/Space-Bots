#pragma once

#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h> // This is to get malloc
#include <string.h> // This is to get memset
#include <sys/stat.h> // Used to get the edit timestamp of files
#include <cmath>
#include <new>
#include <cstring>
#include <typeinfo>

// NOTE: Cross platform stuffs
#ifdef _WIN32
    #define DEBUG_BREAK() __debugbreak()
    #define EXPORT_FN __declspec(dllexport)
    #include <windows.h>
    
    #define BENCHMARK(fn, iterations) do { \
        FILETIME creationTime, exitTime, kernelTime, userTime; \
        GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime); \
        ULARGE_INTEGER start, end; \
        start.LowPart = userTime.dwLowDateTime; \
        start.HighPart = userTime.dwHighDateTime; \
        for (int i = 0; i < iterations; ++i) { fn(); } \
        GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime); \
        end.LowPart = userTime.dwLowDateTime; \
        end.HighPart = userTime.dwHighDateTime; \
        double elapsed = (end.QuadPart - start.QuadPart) / 10.0; \
        printf("%.3f µs\n", elapsed / iterations); \
    } while (0)
#elif __linux__ || __APPLE__
    #define DEBUG_BREAK() __builtin_trap()
    #define EXPORT_FN extern "C"
    #include <ctime>
    
    #define BENCHMARK(fn, iterations) do { \
        clock_t start = clock(); \
        for (int i = 0; i < iterations; ++i) { fn(); } \
        clock_t end = clock(); \
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC * 1e6; \
        printf("%.3f µs\n", elapsed / iterations); \
    } while (0)
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
void _log(const char* prefix, const char* msg, TextColor textColor, Args... args)
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
// #define LOG_TRACE(msg, ...) ((void)0);
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

  ArrayCT() = default;
  ArrayCT(const ArrayCT&) = delete;
  ArrayCT& operator=(const ArrayCT&) = delete;
  ArrayCT(ArrayCT&& other) = delete;
  ArrayCT& operator=(ArrayCT&& other) = delete;

  T& get(int idx) noexcept {
    LOG_ASSERT(idx >= 0 && idx < count, "Index out of bounds!");
    return elements[idx];
  }

  T& operator[](int idx) noexcept {
    return get(idx);
  }

  int add(const T& element) noexcept {
    LOG_ASSERT(count + 1 <= maxElements, "Array Full!");
    elements[count] = element;
    return count++;
  }

  int add(const T* elements_list, int num_elements) noexcept {
    LOG_ASSERT(count + num_elements <= maxElements, "Would overflow array!");
    for (int i = 0; i < num_elements; ++i) {
      add(elements_list[i]);
    }
    return count;
  }

  void remove(int idx) noexcept { //O(1) but doesn't keep order (swap to last index & decrement)
    LOG_ASSERT(idx < count, "idx out of bounds!");
    elements[idx] = elements[--count];
  }

  void ordered_remove(int idx) noexcept {
    LOG_ASSERT(idx < count, "idx out of bounds!");
    for (int i = idx; i < count - 1; ++i) {
      elements[i] = elements[i + 1];
    }
    count--;
  }

  T& front() noexcept {
    LOG_ASSERT(count > 0, "Array is empty!");
    return elements[0];
  }

  T& back() noexcept {
    LOG_ASSERT(count > 0, "Array is empty!");
    return elements[count - 1];
  }

  void reserve(int amount) noexcept {
    LOG_ASSERT(count + amount <= maxElements, "Cannot reserve more than max capacity!");
    for (int i = count; i < count + amount; i++) {
        elements[i] = T{};
    }
    count += amount;
  }

  void init(int amount) noexcept {
    reserve(amount);
  }

  void set_count(int amount) noexcept {
    LOG_ASSERT(amount <= maxElements, "Cannot set count to more than max capacity!");
    count = amount;
  }

  T& pop() noexcept {
    LOG_ASSERT(!empty(), "Cannot pop an empty array");
    count--;
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

  size_t max_elements() const noexcept {
    return maxElements;
  }

  using Iterator = ArrayIterator<T>;
  Iterator begin() noexcept { return Iterator(elements, elements + count); }
  Iterator end() noexcept { return Iterator(elements + count, elements + count); }
};

template<typename T>
struct ArrayRT {
  int capacity;  // runtime capacity, set when allocated
  int count = 0; // number of elements added so far
  T elements[1]; // Flexible array member (allocate extra space)

  ArrayRT() = delete;
  ArrayRT(const ArrayRT&) = delete;
  ArrayRT& operator=(const ArrayRT&) = delete;
  ArrayRT(ArrayRT&& other) = delete;
  ArrayRT& operator=(ArrayRT&& other) = delete;

  T& get(int idx) noexcept {
    LOG_ASSERT(idx >= 0 && idx < count, "Index out of bounds!");
    return elements[idx];
  }

  T& operator[](int idx) noexcept {
    return get(idx);
  }

  int add(const T& element) noexcept {
    LOG_ASSERT(count < capacity, "Array Full!");
    elements[count] = element;
    return count++;
  }

  int add(const T* elements_list, int num_elements) noexcept {
    LOG_ASSERT(count + num_elements <= capacity, "Would overflow array!");
    for (int i = 0; i < num_elements; ++i)
      add(elements_list[i]);
    return count;
  }

  void remove(int idx) noexcept { // O(1): swap with last and decrement
    LOG_ASSERT(idx < count, "Index out of bounds!");
    elements[idx] = elements[--count];
  }

  void ordered_remove(int idx) noexcept {
    LOG_ASSERT(idx < count, "idx out of bounds!");
    for (int i = idx; i < count - 1; ++i) {
      elements[i] = elements[i + 1];
    }
    count--;
  }

  void reserve(int amount) noexcept {
    LOG_ASSERT(count + amount <= capacity, "Cannot reserve more than max capacity!");
    for (int i = count; i < count + amount; i++) {
        elements[i] = T{};
    }
    count += amount;
  }

  void init(int amount) noexcept {
    reserve(amount);
  }

  void set_count(int amount) noexcept {
    LOG_ASSERT(amount <= capacity, "Cannot set count to more than max capacity!");
    count = amount;
  }

  T& pop() noexcept {
    LOG_ASSERT(!empty(), "Cannot pop an empty array");
    count--;
  }

  T& front() noexcept {
    LOG_ASSERT(count > 0, "Array is empty!");
    return elements[0];
  }

  T& back() noexcept {
    LOG_ASSERT(count > 0, "Array is empty!");
    return elements[count - 1];
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

  size_t max_elements() const noexcept {
    return capacity;
  }

  using Iterator = ArrayIterator<T>;
  Iterator begin() noexcept { return Iterator(elements, elements + count); }
  Iterator end() noexcept { return Iterator(elements + count, elements + count); }
};

template<typename T>
void swap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

template<typename T>
int partition(T* arr, int low, int high) {
    T pivot = arr[high];
    int i = low - 1;

    for(int j = low; j < high; j++) {
        if(arr[j] <= pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return i + 1;
}

template<typename T>
void quicksort_internal(T* arr, int low, int high) {
    if(low < high) {
        int pi = partition(arr, low, high);
        quicksort_internal(arr, low, pi - 1);
        quicksort_internal(arr, pi + 1, high);
    }
}

template<typename T, int N>
void quicksort(ArrayCT<T, N>& arr) {
    if(arr.count <= 1) return;
    quicksort_internal(arr.elements, 0, arr.count - 1);
}

template<typename T>
void quicksort(ArrayRT<T>& arr) {
    if(arr.count <= 1) return;
    quicksort_internal(arr.elements, 0, arr.count - 1);
}

template<typename T, int N>
void quicksort(ArrayCT<T, N>& arr, int start, int end) {
    LOG_ASSERT(start >= 0 && end < arr.count, "Index out of bounds!");
    if(end - start <= 0) return;
    quicksort_internal(arr.elements, start, end);
}

template<typename T>
void quicksort(ArrayRT<T>& arr, int start, int end) {
    LOG_ASSERT(start >= 0 && end < arr.count, "Index out of bounds!");
    if(end - start <= 0) return;
    quicksort_internal(arr.elements, start, end);
}

// NOTE: GenArray

struct ID {
  static constexpr uint32_t INDEX_BITS = 24;
  static constexpr uint32_t GEN_BITS = 8;
  static constexpr uint32_t INDEX_MASK = (1 << INDEX_BITS) - 1;
  static constexpr uint32_t GEN_MASK = (1 << GEN_BITS) - 1;

  uint32_t value;  // packed index and generation

  uint32_t index() const { return value & INDEX_MASK; }
  uint32_t generation() const { return (value >> INDEX_BITS) & GEN_MASK; }

  static ID make(uint32_t idx, uint32_t gen) {
    return {(gen << INDEX_BITS) | idx};
  }
};

template<typename T, size_t N>
class GenArrayCT {
public:
  ArrayCT<T, N> entries;
  ArrayCT<uint8_t, N> generations;

  GenArrayCT() = default;
  GenArrayCT(const GenArrayCT&) = delete;
  GenArrayCT& operator=(const GenArrayCT&) = delete;
  GenArrayCT(GenArrayCT&& other) = delete;
  GenArrayCT& operator=(GenArrayCT&& other) = delete;

  ID add(const T& data = T{}) noexcept {
    uint32_t idx = entries.add(data);
    return ID::make(idx, generations[idx]);
  }

  T* get(ID idx) noexcept {
    if (idx.index() >= entries.size()) return nullptr;
    return (generations[idx.index()] == idx.generation()) ? &entries[idx.index()] : nullptr;
  }

  void remove(ID idx) noexcept {
    if (idx.index() >= entries.size()) return;
    if (generations[idx.index()] == idx.generation()) {
      generations[idx.index()]++;
      entries.remove(idx.index());
    }
  }
};

template<typename T>
struct GenArrayRT {
  ArrayRT<T>* entries;
  ArrayRT<uint8_t>* generations;

  GenArrayRT() = default;
  GenArrayRT(const GenArrayRT&) = delete;
  GenArrayRT& operator=(const GenArrayRT&) = delete;
  GenArrayRT(GenArrayRT&& other) = delete;
  GenArrayRT& operator=(GenArrayRT&& other) = delete;

  ID add(const T& data = T{}) noexcept {
    uint32_t idx = entries->add(data);
    return ID::make(idx, generations->get(idx));
  }

  T* get(ID idx) noexcept {
    if (idx.index() >= entries->size()) return nullptr;
    return (generations->get(idx.index()) == idx.generation()) ? &entries->get(idx.index()) : nullptr;
  }

  void remove(ID idx) noexcept {
    if (idx.index() >= entries->size()) return;
    if (generations->get(idx.index()) == idx.generation()) {
      generations->get(idx.index())++;
      entries->remove(idx.index());
    }
  }
};

//NOTE: Map

bool is_string_key(const void* key, size_t size) noexcept;
bool compare_keys(const void* a, const void* b, size_t size) noexcept;

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

    KeyType& key() const { return ptr->key; }
    ValueType& value() const { return ptr->value; }
};

template <typename KeyType, typename ValueType, int Size>
struct MapCT {
  ArrayCT<Entry<KeyType, ValueType>, Size> entries;

  MapCT() = default;
  MapCT(const MapCT&) = delete;
  MapCT& operator=(const MapCT&) = delete;
  MapCT(MapCT&& other) = delete;
  MapCT& operator=(MapCT&& other) = delete;

  // Linear search to find an entry by key
  int find(const KeyType& key) const {
    for (int i = 0; i < entries.count; ++i) {
      if (compare_keys(&entries.elements[i].key, &key, sizeof(KeyType))) {
        return i;
      }
    }
    return -1;
  }

  ValueType& get(const KeyType key) {
    int idx = find(key);
    if (idx == -1) {
      idx = entries.add(Entry<KeyType, ValueType>{key, ValueType{}});
    }
    return entries[idx].value;
  }

  ValueType& operator[](const KeyType key) {
    return get(key);
  }

  void remove(const KeyType& key) {
    int idx = find(key);
    if (idx != -1) {
      entries.remove(idx);
    }
  }

  void ordered_remove(const KeyType& key) {
    int idx = find(key);
    if (idx != -1) {
      entries.ordered_remove(idx);
    }
  }

  bool empty() const noexcept {
    return entries.empty();
  }

  size_t size() const noexcept {
    return entries.size();
  }

  size_t capacity() const noexcept {
    return entries.max_elements();
  }

  bool is_full() const noexcept {
    return size() == capacity();
  }

  bool contains(const KeyType& key) const noexcept {
    return find(key) != -1;
  }

  void clear() noexcept {
    entries.clear();
  }

  using Iterator = MapIterator<KeyType, ValueType>;
  Iterator begin() noexcept { return Iterator(entries.elements, entries.elements + entries.count); }
  Iterator end() noexcept { return Iterator(entries.elements + entries.count, entries.elements + entries.count); }
};

template <typename KeyType, typename ValueType>
struct MapRT {
  ArrayRT<Entry<KeyType, ValueType>>* entries; // set when allocated

  MapRT() = delete;
  MapRT(const MapRT&) = delete;
  MapRT& operator=(const MapRT&) = delete;
  MapRT(MapRT&& other) = delete;
  MapRT& operator=(MapRT&& other) = delete;

  // Linear search to find an entry by key
  int find(const KeyType& key) const {
    for (int i = 0; i < entries->count; ++i) {
      if (compare_keys(&entries->elements[i].key, &key, sizeof(KeyType))) {
        return i;
      }
    }
    return -1;
  }

  ValueType& get(const KeyType key) {
    int idx = find(key);
    if (idx == -1) {
      idx = entries->add(Entry<KeyType, ValueType>{key, ValueType{}});
    }
    return entries->get(idx).value;
  }

  ValueType& operator[](const KeyType key) {
    return get(key);
  }

  void remove(const KeyType& key) {
    int idx = find(key);
    if (idx != -1) {
      entries->remove(idx);
    }
  }

  void ordered_remove(const KeyType& key) {
    int idx = find(key);
    if (idx != -1) {
      entries->ordered_remove(idx);
    }
  }

  bool empty() const noexcept {
    return entries->empty();
  }

  size_t size() const noexcept {
    return entries->size();
  }

  size_t capacity() const noexcept {
    return entries->max_elements();
  }

  bool is_full() const noexcept {
    return size() == capacity();
  }

  bool contains(const KeyType& key) const noexcept {
    return find(key) != -1;
  }

  void clear() noexcept {
    entries->clear();
  }

  using Iterator = MapIterator<KeyType, ValueType>;
  Iterator begin() noexcept { return Iterator(entries->elements, entries->elements + entries->count); }
  Iterator end() noexcept { return Iterator(entries->elements + entries->count, entries->elements + entries->count); }
};

// NOTE: Hashmap

template<typename T>
struct Hash {
  size_t operator()(const T& key) const = delete;
};

template<>
struct Hash<int> {
  size_t operator()(const int& key) const noexcept {
    return key * 2654435761u;
  }
};

template<>
struct Hash<const char*> {
  size_t operator()(const char* key) const noexcept {
    // FNV-1a hash
    size_t hash = 14695981039346656037ULL;
    for(; *key; ++key) {
      hash ^= *key;
      hash *= 1099511628211ULL;
    }
    return hash;
  }
};

enum class EntryState : uint8_t {
  Empty,
  Occupied,
  Dead
};

template<typename K, typename V>
struct HashEntry {
  K key;
  V value;
  EntryState state = EntryState::Empty;
};

template<typename KeyType, typename ValueType, int Size>
struct HashMapCT {
  static constexpr int maxElements = Size;
  static constexpr float maxLoadFactor = 0.7f;
  HashEntry<KeyType, ValueType> entries[Size];
  int count = 0;
  Hash<KeyType> hasher;

  HashMapCT() = default;
  HashMapCT(const HashMapCT&) = delete;
  HashMapCT& operator=(const HashMapCT&) = delete;
  HashMapCT(HashMapCT&& other) = delete;
  HashMapCT& operator=(HashMapCT&& other) = delete;

  int find_slot(const KeyType& key) const noexcept {
    size_t hash = hasher(key);
    size_t idx = hash % Size;
    size_t original = idx;

    do {
      if (entries[idx].state == EntryState::Empty) return -1;
      if (entries[idx].state == EntryState::Occupied && 
        compare_keys(&entries[idx].key, &key, sizeof(KeyType))) {
        return idx;
      }
      idx = (idx + 1) % Size;
    } while (idx != original);

    return -1;
  }

  int find_empty_slot(const KeyType& key) noexcept {
    size_t hash = hasher(key);
    size_t idx = hash % Size;
    size_t original = idx;

    do {
      if (entries[idx].state != EntryState::Occupied) return idx;
      idx = (idx + 1) % Size;
    } while (idx != original);

    LOG_ASSERT(false, "No empty slots!");
    return -1;
  }

  ValueType& get(const KeyType& key) {
    int idx = find_slot(key);
    if (idx == -1) {
      LOG_ASSERT(count < Size * maxLoadFactor, "HashMap too full!");
      idx = find_empty_slot(key);
      entries[idx].key = key;
      entries[idx].value = ValueType{};
      entries[idx].state = EntryState::Occupied;
      count++;
    }
    return entries[idx].value;
  }

  ValueType& operator[](const KeyType& key) {
    return get(key);
  }

  void remove(const KeyType& key) {
    int idx = find_slot(key);
    if (idx != -1) {
      entries[idx].state = EntryState::Dead;
      count--;
    }
  }

  bool contains(const KeyType& key) const noexcept {
    return find_slot(key) != -1;
  }

  size_t size() const noexcept { return count; }

  bool empty() const noexcept { return count == 0; }

  void clear() noexcept {
    for (int i = 0; i < Size; i++) {
      entries[i].state = EntryState::Empty;
    }
    count = 0;
  }
};

template<typename KeyType, typename ValueType>
struct HashMapRT {
  HashEntry<KeyType, ValueType>* entries;
  int capacity;
  int count = 0;
  static constexpr float maxLoadFactor = 0.7f;
  Hash<KeyType> hasher;

  HashMapRT() = delete;
  HashMapRT(const HashMapRT&) = delete;
  HashMapRT& operator=(const HashMapRT&) = delete;
  HashMapRT(HashMapRT&& other) = delete;
  HashMapRT& operator=(HashMapRT&& other) = delete;

  int find_slot(const KeyType& key) const noexcept {
    size_t hash = hasher(key);
    size_t idx = hash % capacity;
    size_t original = idx;

    do {
      if (entries[idx].state == EntryState::Empty) return -1;
      if (entries[idx].state == EntryState::Occupied && 
        compare_keys(&entries[idx].key, &key, sizeof(KeyType))) {
        return idx;
      }
      idx = (idx + 1) % capacity;
    } while (idx != original);

    return -1;
  }

  int find_empty_slot(const KeyType& key) noexcept {
    size_t hash = hasher(key);
    size_t idx = hash % capacity;
    size_t original = idx;

    do {
      if (entries[idx].state != EntryState::Occupied) return idx;
      idx = (idx + 1) % capacity;
    } while (idx != original);

    LOG_ASSERT(false, "No empty slots!");
    return -1;
  }

  ValueType& get(const KeyType& key) {
    int idx = find_slot(key);
    if (idx == -1) {
      LOG_ASSERT(count < capacity * maxLoadFactor, "HashMap too full!");
      idx = find_empty_slot(key);
      entries[idx].key = key;
      entries[idx].value = ValueType{};
      entries[idx].state = EntryState::Occupied;
      count++;
    }
    return entries[idx].value;
  }

  ValueType& operator[](const KeyType& key) {
    return get(key);
  }

  void remove(const KeyType& key) {
    int idx = find_slot(key);
    if (idx != -1) {
      entries[idx].state = EntryState::Dead;
      count--;
    }
  }

  bool contains(const KeyType& key) const noexcept {
    return find_slot(key) != -1;
  }

  size_t size() const noexcept { return count; }

  bool empty() const noexcept { return count == 0; }

  void clear() noexcept {
    for (int i = 0; i < capacity; i++) {
      entries[i].state = EntryState::Empty;
    }
    count = 0;
  }
};

// NOTE: Arena
class Arena {
public:
  size_t capacity;
  size_t used;
  char* memory;

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;
  Arena(Arena&& other) = delete;
  Arena& operator=(Arena&& other) = delete;

  explicit Arena(size_t size) {
    memory = (char*)malloc(size);
    if (!memory) LOG_ASSERT(false, "Failed to allocate memory!");
    memset(memory, 0, size);
    capacity = size;
    used = 0;
  }

  static Arena& create(size_t size) {
    return *new Arena(size);
  }

  char& get(size_t idx) noexcept {
    LOG_ASSERT(idx < used, "Index out of bounds!");
    return memory[idx];
  }

  char& operator[](size_t idx) noexcept {
    return get(idx);
  }

  template<typename T, typename Arg>
  T& create(Arg& arg) noexcept {
    T* ptr = alloc_raw<T>();
    return *(new (ptr) T(arg));
  }

  template<typename T>
  T* alloc_raw() noexcept { // Guaranteed to return valid memory or assert
    size_t size = sizeof(T);
    size_t aligned_size = (size + 7) & ~7;  // 8-byte alignment
    if (used + aligned_size > capacity) LOG_ASSERT(false, "Arena is full");
    T* result = (T*)(memory + used);
    used += aligned_size;
    return result;
  }

  template<typename T>
  T& alloc() noexcept {
    return *alloc_raw<T>();
  }

  template<typename T>
  T* alloc_raw(size_t size) noexcept { // Guaranteed to return valid memory or assert
    size_t aligned_size = (size + 7) & ~7;  // 8-byte alignment
    if (used + aligned_size > capacity) LOG_ASSERT(false, "Arena is full");
    T* result = (T*)(memory + used);
    used += aligned_size;
    return result;
  }

  template<typename T>
  T& alloc(size_t size) noexcept {
    return *alloc_raw<T>(size);
  }

  template<typename T>
  T* alloc_count_raw(size_t count) noexcept { // Guaranteed to return valid memory or assert
    size_t total_size = sizeof(T) * count;
    size_t aligned_size = (total_size + 7) & ~7;  // 8-byte alignment
    if (used + aligned_size > capacity) LOG_ASSERT(false, "Arena is full");
    T* result = reinterpret_cast<T*>(memory + used);
    used += aligned_size;
    return result;
  }

  template<typename T>
  T& alloc_count(size_t count) noexcept {
    return *alloc_count_raw<T>(count);
  }

  template <typename E, typename M> 
  E& fetch(const char* key) {
    M* map_ptr = reinterpret_cast<M*>(memory);
    void* ptr = map_ptr->get(key);
    return *reinterpret_cast<E*>(ptr);
  }

  template<typename T>
  ArrayRT<T>& create_array_rt(size_t capacity) {
    size_t total_size = sizeof(ArrayRT<T>) + sizeof(T) * (capacity - 1);
    ArrayRT<T>& arr = alloc<ArrayRT<T>>(total_size);
    arr.capacity = capacity;
    arr.clear();
    return arr;
  }

  template<typename T, int N>
  ArrayCT<T, N>& create_array_ct() {
    ArrayCT<T, N>& arr = alloc<ArrayCT<T, N>>();
    arr.clear();
    return arr;
  }

  template<typename KeyType, typename ValueType>
  MapRT<KeyType, ValueType>& create_map_rt(size_t capacity) {
    MapRT<KeyType, ValueType>& map = alloc<MapRT<KeyType, ValueType>>(sizeof(MapRT<KeyType, ValueType>));
    ArrayRT<Entry<KeyType, ValueType>>& entries = create_array_rt<Entry<KeyType, ValueType>>(capacity);
    map.entries = &entries;
    map.clear();
    return map;
  }

  template<typename KeyType, typename ValueType, int N>
  MapCT<KeyType, ValueType, N>& create_map_ct() {
    MapCT<KeyType, ValueType, N>& map = alloc<MapCT<KeyType, ValueType, N>>();
    map.clear();
    return map;
  }

  template<typename KeyType, typename ValueType>
  HashMapRT<KeyType, ValueType>& create_hashmap_rt(size_t capacity) {
    HashMapRT<KeyType, ValueType>& map = alloc<HashMapRT<KeyType, ValueType>>();
    HashEntry<KeyType, ValueType>* entries = alloc_count_raw<HashEntry<KeyType, ValueType>>(capacity);
    map.entries = entries;
    map.capacity = capacity;
    map.clear();
    return map;
  }

  template<typename KeyType, typename ValueType, int N>
  HashMapCT<KeyType, ValueType, N>& create_hashmap_ct() {
    HashMapCT<KeyType, ValueType, N>& map = alloc<HashMapCT<KeyType, ValueType, N>>();
    map.clear();
    return map;
  }

  template<typename T>
  GenArrayRT<T>& create_genarray_rt(size_t capacity) {
    LOG_ASSERT(capacity <= (1 << ID::INDEX_BITS), "GenArray size cannot exceed 16M elements");
    GenArrayRT<T>& arr = alloc<GenArrayRT<T>>();
    arr.entries = &create_array_rt<T>(capacity);
    arr.generations = &create_array_rt<uint8_t>(capacity);
    arr.generations->reserve(capacity);
    return arr;
  }

  template<typename T, int N>
  GenArrayCT<T, N>& create_genarray_ct() {
    LOG_ASSERT(N <= (1 << ID::INDEX_BITS), "GenArray size cannot exceed 16M elements");
    GenArrayCT<T, N>& arr = alloc<GenArrayCT<T, N>>();
    arr.generations.reserve(N);
    return arr;
  }

  void clear() noexcept {
    used = 0;
    memset(memory, 0, capacity); // Sets the memory to 0
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

inline Arena& create_arena(size_t size) {
    return Arena::create(size);
}

// NOTE: Size defs
#define KB(x) ((x) * 1024ULL)
#define MB(x) ((x) * 1024ULL * 1024ULL)
#define GB(x) ((x) * 1024ULL * 1024ULL * 1024ULL)

// NOTE: File I/O
long long get_timestamp(const char* file) noexcept;
bool file_exists(const char* filePath) noexcept;
size_t get_file_size(const char* filePath) noexcept;
char* read_file(const char* filePath, Arena& arena) noexcept;
void write_file(const char* filePath, const char* buffer, size_t size) noexcept;
bool copy_file(const char* fileName, const char* outputName, Arena& arena) noexcept;
void remove_file(const char* fileName) noexcept;

//NOTE: Testing
bool CompareFloat(float a, float b, float epsilon = 0.0001f) noexcept;
bool CompareIntArrays(const int *a, const int *b, size_t size) noexcept;
bool CompareFloatArrays(const float *a, const float *b, size_t size) noexcept;
bool CompareUCharArrays(const unsigned char *a, const unsigned char *b, size_t size) noexcept;
bool CompareUIntArrays(const unsigned int *a, const unsigned int *b, size_t size) noexcept;
bool CompareUShortArrays(const unsigned short *a, const unsigned short *b, size_t size) noexcept;
