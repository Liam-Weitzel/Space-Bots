#pragma once

#include <cctype>
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

  void set_reserve(int amount) noexcept {
    LOG_ASSERT(amount <= maxElements, "Cannot set count to more than max capacity!");
    for (int i = 0; i < amount; i++) {
        elements[i] = T{};
    }
    count = amount;
  }

  T& pop() noexcept {
    LOG_ASSERT(!empty(), "Cannot pop an empty array");
    count--;
  }

  int find(const T& value) const noexcept {
    for (uint32_t i = 0; i < count; i++) {
      if (elements[i] == value) {
        return i;
      }
    }
    return -1;
  }

  bool contains(const T& value) const noexcept {
    return find(value) != -1;
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

  void set_reserve(int amount) noexcept {
    LOG_ASSERT(amount <= capacity, "Cannot set count to more than max capacity!");
    for (int i = 0; i < amount; i++) {
        elements[i] = T{};
    }
    count = amount;
  }

  T& pop() noexcept {
    LOG_ASSERT(!empty(), "Cannot pop an empty array");
    count--;
  }

  int find(const T& value) const noexcept {
    for (uint32_t i = 0; i < count; i++) {
      if (elements[i] == value) {
        return i;
      }
    }
    return -1;
  }

  bool contains(const T& value) const noexcept {
    return find(value) != -1;
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

//NOTE: Map

template<typename K>
struct KeyCompare {
  static bool equals(const K& a, const K& b) noexcept {
    // Handle char* types
    if (sizeof(K) == sizeof(char*)) {
      const char* str_a = (const char*)a;
      const char* str_b = (const char*)b;
      return str_a && str_b && strcmp(str_a, str_b) == 0;
    }
    // Handle char arrays
    else if (sizeof(K) > 1 && ((const char*)&a)[0] != '\0') {
      return strcmp((const char*)&a, (const char*)&b) == 0;
    }
    // Handle 32-bit types
    else if (sizeof(K) == sizeof(uint32_t)) {
      return a == b;
    }
    // Handle 64-bit types
    else if (sizeof(K) == sizeof(uint64_t)) {
      return a == b;
    }
    // Default case
    return memcmp(&a, &b, sizeof(K)) == 0;
  }
};

template <typename KeyType, typename ValueType>
struct Entry {
  KeyType key;
  ValueType value;

  bool operator==(const Entry& other) const noexcept {
    return KeyCompare<KeyType>::equals(key, other.key);
  }
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
    Entry<KeyType, ValueType> temp{key, ValueType{}};
    return entries.find(temp);
  }

  bool contains(const KeyType& key) const noexcept {
    Entry<KeyType, ValueType> temp{key, ValueType{}};
    return entries.contains(temp);
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
    Entry<KeyType, ValueType> temp{key, ValueType{}};
    return entries->find(temp);
  }

  bool contains(const KeyType& key) const noexcept {
    Entry<KeyType, ValueType> temp{key, ValueType{}};
    return entries->contains(temp);
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

  bool operator==(const HashEntry& other) const noexcept {
    return KeyCompare<K>::equals(key, other.key);
  }
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
        KeyCompare<KeyType>::equals(entries[idx].key, key)) {
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
        KeyCompare<KeyType>::equals(entries[idx].key, key)) {
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

// NOTE: Generational Sparse set

struct GenId {
  static constexpr uint32_t GEN_BITS = 8;
  static constexpr uint32_t ID_BITS = 24;
  static constexpr uint32_t ID_MASK = (1u << ID_BITS) - 1;
  static constexpr uint32_t GEN_MASK = ((1u << GEN_BITS) - 1) << ID_BITS;

  uint32_t packed;

  bool operator==(const GenId& other) const {
    return packed == other.packed;
  }

  bool operator!=(const GenId& other) const {
    return packed != other.packed;
  }

  static GenId create(uint32_t id, uint8_t gen) {
    LOG_ASSERT(id < (1u << ID_BITS), "ID exceeds 24 bits");
    return GenId{(gen << ID_BITS) | id};
  }

  void set_id(uint32_t id) {
    LOG_ASSERT(id < (1u << ID_BITS), "ID exceeds 24 bits");
    packed = (packed & ~ID_MASK) | id;
  }

  void set_gen(uint8_t gen) {
    packed = (packed & ID_MASK) | (gen << ID_BITS);
  }

  void increment_gen() {
    uint8_t next_gen = (gen() + 1) & ((1 << GEN_BITS) - 1);
    set_gen(next_gen);
  }

  uint32_t id() const { return packed & ID_MASK; }

  uint8_t gen() const { return (packed >> ID_BITS) & ((1u << GEN_BITS) - 1); }
};

template<typename T, size_t N>
struct GenSparseSetCT {
  ArrayCT<T, N> dense;
  ArrayCT<GenId, N> sparse;
  uint32_t free_head;

  void init() {
    sparse.set_reserve(N);
    for (uint32_t i = 0; i < N-1; i++) {
      auto& entry = sparse[i];
      entry.set_id(i + 1);
      entry.set_gen(0);
    }
    auto& last = sparse[N-1];
    last.set_id(N);
    last.set_gen(0);
    free_head = 0;
  }

  GenId add(const T& val) {
    auto& entry = sparse[free_head];
    uint32_t next_free = entry.id();
    uint32_t dense_idx = dense.add(val);

    entry.set_id(dense_idx);
    free_head = next_free;

    return entry;
  }

  void remove(GenId genId) {
    if (genId.id() >= N) return;

    auto& entry = sparse[genId.id()];
    if (entry != genId) return;

    dense.remove(entry.id());
    entry.set_id(free_head);
    entry.increment_gen();
    free_head = genId.id();
  }

  T* get(GenId genId) {
    if (genId.id() >= N) return nullptr;
    const auto& entry = sparse[genId.id()];
    if (entry.id() >= dense.size()) return nullptr;
    if (entry != genId) return nullptr;
    return &dense[entry.id()];
  }

  bool contains(GenId genId) {
    const auto& entry = sparse[genId.id()];
    if (entry.id() >= dense.size()) return false;
    return entry == genId;
  }

  void clear() {
    dense.clear();
    sparse.set_reserve(N);

    // Rebuild free list while preserving generations
    for (uint32_t i = 0; i < N-1; i++) {
      auto& entry = sparse[i];
      uint8_t gen = entry.gen();  // preserve generation
      entry.set_id(i + 1);        // rebuild free list
    }

    auto& last = sparse[N-1];
    uint8_t gen = last.gen();  // preserve generation
    last.set_id(N);

    free_head = 0;
  }

  size_t size() const { return dense.size(); }

  bool empty() const { return dense.empty(); }

  using Iterator = typename ArrayCT<T, N>::Iterator;
  Iterator begin() { return dense.begin(); }
  Iterator end() { return dense.end(); }
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

  template<typename T, size_t N>
  GenSparseSetCT<T, N>& create_gen_sparse_set_ct() {
    GenSparseSetCT<T, N>& genSparseSet = alloc<GenSparseSetCT<T, N>>();
    genSparseSet.init();
    return genSparseSet;
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
