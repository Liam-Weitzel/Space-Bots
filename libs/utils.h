#pragma once

#include <algorithm>
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
        for (uint32_t i = 0; i < iterations; ++i) { fn(); } \
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
// #define LOG_TRACE(msg, ...) ((void)0); // Uncomment this for performance testing
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

template<typename T, uint32_t N>
struct ArrayCT {
  static constexpr uint32_t maxElements = N;
  uint32_t count = 0;
  T elements[maxElements];

  ArrayCT() = default;
  ArrayCT(const ArrayCT&) = delete;
  ArrayCT& operator=(const ArrayCT&) = delete;
  ArrayCT(ArrayCT&& other) = delete;
  ArrayCT& operator=(ArrayCT&& other) = delete;

  void init() {
    clear();
  }

  T& get(uint32_t idx) {
    LOG_ASSERT(idx < count, "Index out of bounds!");
    return elements[idx];
  }

  const T& get(uint32_t idx) const {
    LOG_ASSERT(idx < count, "Index out of bounds!");
    return elements[idx];
  }

  T& operator[](uint32_t idx) {
    return get(idx);
  }

  const T& operator[](uint32_t idx) const {
    return get(idx);
  }

  uint32_t add(const T& element) {
    LOG_ASSERT(count + 1 <= maxElements, "Array Full!");
    elements[count] = element;
    return count++;
  }

  uint32_t add(const T* elements_list, uint32_t num_elements) {
    LOG_ASSERT(count + num_elements <= maxElements, "Would overflow array!");
    for (uint32_t i = 0; i < num_elements; ++i) {
      add(elements_list[i]);
    }
    return count;
  }

  void remove(uint32_t idx) { //O(1) but doesn't keep order (swap to last index & decrement)
    LOG_ASSERT(idx < count, "idx out of bounds!");
    elements[idx] = elements[--count];
  }

  void ordered_remove(uint32_t idx) {
    LOG_ASSERT(idx < count, "idx out of bounds!");
    for (uint32_t i = idx; i < count - 1; ++i) {
      elements[i] = elements[i + 1];
    }
    count--;
  }

  T& front() {
    LOG_ASSERT(count > 0, "Array is empty!");
    return elements[0];
  }

  T& back() {
    LOG_ASSERT(count > 0, "Array is empty!");
    return elements[count - 1];
  }

  void reserve(uint32_t amount) {
    LOG_ASSERT(count + amount <= maxElements, "Cannot reserve more than max capacity!");
    for (uint32_t i = count; i < count + amount; i++) {
        elements[i] = T{};
    }
    count += amount;
  }

  void reserve_until(uint32_t amount) {
    LOG_ASSERT(amount <= maxElements, "Cannot set count to more than max capacity!");
    for (uint32_t i = count; i < amount; i++) {
        elements[i] = T{};
    }
    count = amount;
  }

  void pop() {
    LOG_ASSERT(!empty(), "Cannot pop an empty array");
    count--;
  }

  uint32_t find(const T& value) const {
    for (uint32_t i = 0; i < count; i++) {
      if (elements[i] == value) {
        return i;
      }
    }
    return UINT32_MAX;
  }

  bool contains(const T& value) const {
    return find(value) != UINT32_MAX;
  }

  void clear() {
    count = 0;
  }

  bool is_full() const {
    return count == N;
  }

  bool empty() const {
      return count == 0;
  }

  uint32_t size() const {
      return count;
  }

  uint32_t capacity() const {
    return maxElements;
  }

  using Iterator = ArrayIterator<T>;
  Iterator begin() { return Iterator(elements, elements + count); }
  Iterator end() { return Iterator(elements + count, elements + count); }
};

template<typename T>
struct ArrayRT {
  uint32_t maxElements;  // runtime capacity, set when allocated
  uint32_t count = 0;    // number of elements added so far
  T elements[1];         // Flexible array member (allocate extra space)

  ArrayRT() = delete;
  ArrayRT(const ArrayRT&) = delete;
  ArrayRT& operator=(const ArrayRT&) = delete;
  ArrayRT(ArrayRT&& other) = delete;
  ArrayRT& operator=(ArrayRT&& other) = delete;

  void init(uint32_t AmaxElements) {
    maxElements = AmaxElements;
    clear();
  }

  T& get(uint32_t idx) {
    LOG_ASSERT(idx < count, "Index out of bounds!");
    return elements[idx];
  }

  const T& get(uint32_t idx) const {
    LOG_ASSERT(idx < count, "Index out of bounds!");
    return elements[idx];
  }

  T& operator[](uint32_t idx) {
    return get(idx);
  }

  const T& operator[](uint32_t idx) const {
    return get(idx);
  }

  uint32_t add(const T& element) {
    LOG_ASSERT(count < maxElements, "Array Full!");
    elements[count] = element;
    return count++;
  }

  uint32_t add(const T* elements_list, uint32_t num_elements) {
    LOG_ASSERT(count + num_elements <= maxElements, "Would overflow array!");
    for (uint32_t i = 0; i < num_elements; ++i)
      add(elements_list[i]);
    return count;
  }

  void remove(uint32_t idx) { // O(1): swap with last and decrement
    LOG_ASSERT(idx < count, "Index out of bounds!");
    elements[idx] = elements[--count];
  }

  void ordered_remove(uint32_t idx) {
    LOG_ASSERT(idx < count, "idx out of bounds!");
    for (uint32_t i = idx; i < count - 1; ++i) {
      elements[i] = elements[i + 1];
    }
    count--;
  }

  void reserve(uint32_t amount) {
    LOG_ASSERT(count + amount <= maxElements, "Cannot reserve more than max capacity!");
    for (uint32_t i = count; i < count + amount; i++) {
        elements[i] = T{};
    }
    count += amount;
  }

  void reserve_until(uint32_t amount) {
    reserve(amount - count);
  }

  void pop() {
    LOG_ASSERT(!empty(), "Cannot pop an empty array");
    count--;
  }

  uint32_t find(const T& value) const {
    for (uint32_t i = 0; i < count; i++) {
      if (elements[i] == value) {
        return i;
      }
    }
    return UINT32_MAX;
  }

  bool contains(const T& value) const {
    return find(value) != UINT32_MAX;
  }

  T& front() {
    LOG_ASSERT(count > 0, "Array is empty!");
    return elements[0];
  }

  T& back() {
    LOG_ASSERT(count > 0, "Array is empty!");
    return elements[count - 1];
  }

  void clear() {
    count = 0;
  }

  bool is_full() const {
    return count == maxElements;
  }

  bool empty() const {
      return count == 0;
  }

  uint32_t size() const {
      return count;
  }

  uint32_t capacity() const {
    return maxElements;
  }

  using Iterator = ArrayIterator<T>;
  Iterator begin() { return Iterator(elements, elements + count); }
  Iterator end() { return Iterator(elements + count, elements + count); }
};

template<typename T>
void swap(T& a, T& b) {
  T temp = a;
  a = b;
  b = temp;
}

template<typename T>
uint32_t partition(T* arr, uint32_t low, uint32_t high) {
  T pivot = arr[high];
  uint32_t i = low - 1;

  for(uint32_t j = low; j < high; j++) {
    if(arr[j] <= pivot) {
      i++;
      swap(arr[i], arr[j]);
    }
  }
  swap(arr[i + 1], arr[high]);
  return i + 1;
}

template<typename T>
void quicksort_internal(T* arr, uint32_t low, uint32_t high) {
  if(low < high) {
    uint32_t pi = partition(arr, low, high);
    quicksort_internal(arr, low, pi - 1);
    quicksort_internal(arr, pi + 1, high);
  }
}

template<typename T, uint32_t N>
void quicksort(ArrayCT<T, N>& arr) {
  if(arr.count <= 1) return;
  quicksort_internal(arr.elements, 0, arr.count - 1);
}

template<typename T>
void quicksort(ArrayRT<T>& arr) {
  if(arr.count <= 1) return;
  quicksort_internal(arr.elements, 0, arr.count - 1);
}

template<typename T, uint32_t N>
void quicksort(ArrayCT<T, N>& arr, uint32_t start, uint32_t end) {
  LOG_ASSERT(end < arr.count, "Index out of bounds!");
  if(end - start <= 0) return;
  quicksort_internal(arr.elements, start, end);
}

template<typename T>
void quicksort(ArrayRT<T>& arr, uint32_t start, uint32_t end) {
  LOG_ASSERT(start >= 0 && end < arr.count, "Index out of bounds!");
  if(end - start <= 0) return;
  quicksort_internal(arr.elements, start, end);
}

//NOTE: Map

template<typename K>
struct KeyCompare {
  static bool equals(const K& a, const K& b) {
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

  bool operator==(const Entry& other) const {
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

template <typename KeyType, typename ValueType, uint32_t N>
struct MapCT {
  ArrayCT<Entry<KeyType, ValueType>, N> entries;

  MapCT() = default;
  MapCT(const MapCT&) = delete;
  MapCT& operator=(const MapCT&) = delete;
  MapCT(MapCT&& other) = delete;
  MapCT& operator=(MapCT&& other) = delete;

  void init() {
    clear();
  }

  // Linear search to find an entry by key
  uint32_t find(const KeyType& key) const {
    Entry<KeyType, ValueType> temp{key, ValueType{}};
    return entries.find(temp);
  }

  bool contains(const KeyType& key) const {
    Entry<KeyType, ValueType> temp{key, ValueType{}};
    return entries.contains(temp);
  }

  ValueType& get(const KeyType key) {
    uint32_t idx = find(key);
    if (idx == UINT32_MAX) {
      idx = entries.add(Entry<KeyType, ValueType>{key, ValueType{}});
    }
    return entries[idx].value;
  }

  ValueType& operator[](const KeyType key) {
    return get(key);
  }

  void remove(const KeyType& key) {
    uint32_t idx = find(key);
    if (idx != UINT32_MAX) {
      entries.remove(idx);
    }
  }

  void ordered_remove(const KeyType& key) {
    uint32_t idx = find(key);
    if (idx != UINT32_MAX) {
      entries.ordered_remove(idx);
    }
  }

  bool empty() const {
    return entries.empty();
  }

  uint32_t size() const {
    return entries.size();
  }

  uint32_t capacity() const {
    return entries.capacity();
  }

  bool is_full() const {
    return size() == capacity();
  }

  void clear() {
    entries.clear();
  }

  using Iterator = MapIterator<KeyType, ValueType>;
  Iterator begin() { return Iterator(entries.elements, entries.elements + entries.count); }
  Iterator end() { return Iterator(entries.elements + entries.count, entries.elements + entries.count); }
};

template <typename KeyType, typename ValueType>
struct MapRT {
  ArrayRT<Entry<KeyType, ValueType>>* entries; // set when allocated

  MapRT() = delete;
  MapRT(const MapRT&) = delete;
  MapRT& operator=(const MapRT&) = delete;
  MapRT(MapRT&& other) = delete;
  MapRT& operator=(MapRT&& other) = delete;

  void init(ArrayRT<Entry<KeyType, ValueType>>& Aentries) {
    entries = &Aentries;
    clear();
  }

  // Linear search to find an entry by key
  uint32_t find(const KeyType& key) const {
    Entry<KeyType, ValueType> temp{key, ValueType{}};
    return entries->find(temp);
  }

  bool contains(const KeyType& key) const {
    Entry<KeyType, ValueType> temp{key, ValueType{}};
    return entries->contains(temp);
  }

  ValueType& get(const KeyType key) {
    uint32_t idx = find(key);
    if (idx == UINT32_MAX) {
      idx = entries->add(Entry<KeyType, ValueType>{key, ValueType{}});
    }
    return entries->get(idx).value;
  }

  ValueType& operator[](const KeyType key) {
    return get(key);
  }

  void remove(const KeyType& key) {
    uint32_t idx = find(key);
    if (idx != UINT32_MAX) {
      entries->remove(idx);
    }
  }

  void ordered_remove(const KeyType& key) {
    uint32_t idx = find(key);
    if (idx != UINT32_MAX) {
      entries->ordered_remove(idx);
    }
  }

  bool empty() const {
    return entries->empty();
  }

  uint32_t size() const {
    return entries->size();
  }

  uint32_t capacity() const {
    return entries->capacity();
  }

  bool is_full() const {
    return size() == capacity();
  }

  void clear() {
    entries->clear();
  }

  using Iterator = MapIterator<KeyType, ValueType>;
  Iterator begin() { return Iterator(entries->elements, entries->elements + entries->count); }
  Iterator end() { return Iterator(entries->elements + entries->count, entries->elements + entries->count); }
};

// NOTE: Hashmap

template<typename T>
struct Hash {
  uint32_t operator()(const T& key) const = delete;
};

template<>
struct Hash<int> {
  uint32_t operator()(const int& key) const {
    return key * 2654435761u;
  }
};

template<>
struct Hash<const char*> {
  unsigned long long operator()(const char* key) const {
    // FNV-1a hash
    unsigned long long hash = 14695981039346656037ULL;
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

  bool operator==(const HashEntry& other) const {
    return KeyCompare<K>::equals(key, other.key);
  }
};

template<typename K, typename V>
struct HashMapIterator {
  HashEntry<K,V>* ptr;
  HashEntry<K,V>* end;

  HashMapIterator(const HashMapIterator&) = delete;
  HashMapIterator& operator=(const HashMapIterator&) = delete;
  HashMapIterator(HashMapIterator&& other) = delete;
  HashMapIterator& operator=(HashMapIterator&& other) = delete;

  HashMapIterator(HashEntry<K,V>* start, HashEntry<K,V>* end) 
      : ptr(start), end(end) {
    // Find first occupied entry
    while (ptr != end && ptr->state != EntryState::Occupied) {
      ++ptr;
    }
  }

  HashMapIterator& operator++() {
    if (ptr != end) {
      ++ptr;
      // Skip to next occupied entry
      while (ptr != end && ptr->state != EntryState::Occupied) {
        ++ptr;
      }
    }
    return *this;
  }

  bool operator!=(const HashMapIterator& other) const { return ptr != other.ptr; }
  HashEntry<K,V>& operator*() const { return *ptr; }
  HashEntry<K,V>* operator->() const { return ptr; }

  K& key() const { return ptr->key; }
  V& value() const { return ptr->value; }
};

template<typename KeyType, typename ValueType, uint32_t N>
struct HashMapCT {
  static constexpr uint32_t maxElements = N;
  static constexpr float maxLoadFactor = 0.7f;
  ArrayCT<HashEntry<KeyType, ValueType>, N> entries;
  uint32_t count = 0;
  Hash<KeyType> hasher;

  HashMapCT() = default;
  HashMapCT(const HashMapCT&) = delete;
  HashMapCT& operator=(const HashMapCT&) = delete;
  HashMapCT(HashMapCT&& other) = delete;
  HashMapCT& operator=(HashMapCT&& other) = delete;

  void init() {
    entries.reserve_until(N);
    clear();
  }

  uint32_t find_slot(const KeyType& key) const {
    uint32_t hash = hasher(key);
    uint32_t idx = hash % N;
    uint32_t original = idx;

    do {
      if (entries[idx].state == EntryState::Empty) return UINT32_MAX;
      if (entries[idx].state == EntryState::Occupied && 
        KeyCompare<KeyType>::equals(entries[idx].key, key)) {
        return idx;
      }
      idx = (idx + 1) % N;
    } while (idx != original);

    return UINT32_MAX;
  }

  uint32_t find_empty_slot(const KeyType& key) {
    uint32_t hash = hasher(key);
    uint32_t idx = hash % N;
    uint32_t original = idx;

    do {
      if (entries[idx].state != EntryState::Occupied) return idx;
      idx = (idx + 1) % N;
    } while (idx != original);

    LOG_ASSERT(false, "No empty slots!");
    return UINT32_MAX;
  }

  ValueType& get(const KeyType& key) {
    uint32_t idx = find_slot(key);
    if (idx == UINT32_MAX) {
      LOG_ASSERT(count < N * maxLoadFactor, "HashMap too full!");
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
    uint32_t idx = find_slot(key);
    if (idx != UINT32_MAX) {
      entries[idx].state = EntryState::Dead;
      count--;
    }
  }

  bool contains(const KeyType& key) const {
    return find_slot(key) != UINT32_MAX;
  }

  uint32_t size() const { return count; }

  bool empty() const { return count == 0; }

  void clear() {
    for (uint32_t i = 0; i < N; i++) {
      entries[i].state = EntryState::Empty;
    }
    count = 0;
  }

  uint32_t capacity() const { return maxElements; }

  bool is_full() const { return size() == capacity(); }

  using Iterator = HashMapIterator<KeyType, ValueType>;
  Iterator begin() { 
    return Iterator(entries.elements, entries.elements + N); 
  }
  Iterator end() { 
    return Iterator(entries.elements + N, entries.elements + N); 
  }
};

template<typename KeyType, typename ValueType>
struct HashMapRT {
  ArrayRT<HashEntry<KeyType, ValueType>>* entries; // Set at runtime
  uint32_t maxElements; // Set at runtime
  uint32_t count = 0;
  static constexpr float maxLoadFactor = 0.7f;
  Hash<KeyType> hasher;

  HashMapRT() = delete;
  HashMapRT(const HashMapRT&) = delete;
  HashMapRT& operator=(const HashMapRT&) = delete;
  HashMapRT(HashMapRT&& other) = delete;
  HashMapRT& operator=(HashMapRT&& other) = delete;

  void init(ArrayRT<HashEntry<KeyType, ValueType>>& Aentries, uint32_t AmaxElements) {
    Aentries.reserve_until(AmaxElements);
    entries = &Aentries;
    maxElements = AmaxElements;
    clear();
  }

  uint32_t find_slot(const KeyType& key) const {
    uint32_t hash = hasher(key);
    uint32_t idx = hash % maxElements;
    uint32_t original = idx;

    do {
      if (entries->get(idx).state == EntryState::Empty) return UINT32_MAX;
      if (entries->get(idx).state == EntryState::Occupied && 
        KeyCompare<KeyType>::equals(entries->get(idx).key, key)) {
        return idx;
      }
      idx = (idx + 1) % maxElements;
    } while (idx != original);

    return UINT32_MAX;
  }

  uint32_t find_empty_slot(const KeyType& key) {
    uint32_t hash = hasher(key);
    uint32_t idx = hash % maxElements;
    uint32_t original = idx;

    do {
      if (entries->get(idx).state != EntryState::Occupied) return idx;
      idx = (idx + 1) % maxElements;
    } while (idx != original);

    LOG_ASSERT(false, "No empty slots!");
    return UINT32_MAX;
  }

  ValueType& get(const KeyType& key) {
    uint32_t idx = find_slot(key);
    if (idx == UINT32_MAX) {
      LOG_ASSERT(count < maxElements * maxLoadFactor, "HashMap too full!");
      idx = find_empty_slot(key);
      entries->get(idx).key = key;
      entries->get(idx).value = ValueType{};
      entries->get(idx).state = EntryState::Occupied;
      count++;
    }
    return entries->get(idx).value;
  }

  ValueType& operator[](const KeyType& key) {
    return get(key);
  }

  void remove(const KeyType& key) {
    uint32_t idx = find_slot(key);
    if (idx != UINT32_MAX) {
      entries->get(idx).state = EntryState::Dead;
      count--;
    }
  }

  bool contains(const KeyType& key) const {
    return find_slot(key) != UINT32_MAX;
  }

  uint32_t size() const { return count; }

  bool empty() const { return count == 0; }

  void clear() {
    for (uint32_t i = 0; i < maxElements; i++) {
      entries->get(i).state = EntryState::Empty;
    }
    count = 0;
  }

  uint32_t capacity() const { return maxElements; }

  bool is_full() const { return size() == capacity(); }

  using Iterator = HashMapIterator<KeyType, ValueType>;
  Iterator begin() { 
    return Iterator(entries->elements, entries->elements + maxElements); 
  }
  Iterator end() { 
    return Iterator(entries->elements + maxElements, entries->elements + maxElements); 
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
    return !(*this==other);
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

template<typename T, uint32_t N>
struct GenSparseSetCT {
  ArrayCT<T, N> dense;
  ArrayCT<GenId, N> sparse;
  ArrayCT<uint32_t, N> dense_to_sparse;
  uint32_t free_head;

  void init() {
    sparse.reserve_until(N);
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
    uint32_t dense_idx = dense.size();

    dense.add(val);
    dense_to_sparse.add(free_head);
    entry.set_id(dense_idx);

    GenId res = GenId::create(free_head, entry.gen());
    free_head = next_free;

    return res;
  }

  void remove(GenId genId) {
    if (genId.id() >= N) return;

    auto& entry = sparse[genId.id()];
    if (entry.gen() != genId.gen()) return;
    uint32_t prev_back = dense_to_sparse.back();

    dense.remove(entry.id());
    dense_to_sparse.remove(entry.id());
    sparse[prev_back].set_id(entry.id());

    entry.set_id(free_head);
    entry.increment_gen();
    free_head = genId.id();
  }

  T* get(GenId genId) {
    if (genId.id() >= N) return nullptr;
    const auto& entry = sparse[genId.id()];
    if (entry.id() >= dense.size()) return nullptr;
    if (entry.gen() != genId.gen()) return nullptr;
    return &dense[entry.id()];
  }

  T* operator[](GenId genId) {
    return get(genId);
  }

  GenId find(const T& value) {
    for (uint32_t i = 0; i < dense.size(); i++) {
      if (dense[i] == value) {
        uint32_t sparse_idx = dense_to_sparse[i];
        return GenId::create(sparse_idx, sparse[sparse_idx].gen());
      }
    }
    return {UINT32_MAX};
  }

  bool contains(GenId genId) {
    if (genId.id() >= sparse.size()) return false;
    const auto& entry = sparse[genId.id()];
    if (entry.id() >= dense.size()) return false;
    return entry.gen() == genId.gen();
  }

  void clear() {
    dense.clear();
    dense_to_sparse.clear();

    // Rebuild free list while preserving generations
    for (uint32_t i = 0; i < N-1; i++) {
      auto& entry = sparse[i];
      entry.increment_gen();
      entry.set_id(i + 1);
    }

    auto& last = sparse[N-1];
    last.increment_gen();
    last.set_id(N);

    free_head = 0;
  }

  uint32_t size() const { return dense.size(); }

  bool empty() const { return dense.empty(); }

  bool is_full() const { return dense.size() == dense.capacity(); }

  bool capacity() const { return dense.capacity(); }

  using Iterator = typename ArrayCT<T, N>::Iterator;
  Iterator begin() { return dense.begin(); }
  Iterator end() { return dense.end(); }
};

template<typename T>
struct GenSparseSetRT {
  uint32_t N; // Set at runtime
  ArrayRT<T>* dense; // Set at runtime
  ArrayRT<GenId>* sparse; // Set at runtime
  ArrayRT<uint32_t>* dense_to_sparse; // Set at runtime
  uint32_t free_head;

  void init(uint32_t _N, ArrayRT<T>& _dense, ArrayRT<GenId>& _sparse, ArrayRT<uint32_t>& _dense_to_sparse) {
    N = _N;
    dense = &_dense;
    sparse = &_sparse;
    dense_to_sparse = &_dense_to_sparse;

    sparse->reserve_until(N);
    for (uint32_t i = 0; i < N-1; i++) {
      auto& entry = sparse->get(i);
      entry.set_id(i + 1);
      entry.set_gen(0);
    }
    auto& last = sparse->get(N-1);
    last.set_id(N);
    last.set_gen(0);
    free_head = 0;
  }

  GenId add(const T& val) {
    auto& entry = sparse->get(free_head);
    uint32_t next_free = entry.id();
    uint32_t dense_idx = dense->size();

    dense->add(val);
    dense_to_sparse->add(free_head);
    entry.set_id(dense_idx);

    GenId res = GenId::create(free_head, entry.gen());
    free_head = next_free;

    return res;
  }

  void remove(GenId genId) {
    if (genId.id() >= N) return;

    auto& entry = sparse->get(genId.id());
    if (entry.gen() != genId.gen()) return;
    uint32_t prev_back = dense_to_sparse->back();

    dense->remove(entry.id());
    dense_to_sparse->remove(entry.id());
    sparse->get(prev_back).set_id(entry.id());

    entry.set_id(free_head);
    entry.increment_gen();
    free_head = genId.id();
  }

  T* get(GenId genId) {
    if (genId.id() >= N) return nullptr;
    const auto& entry = sparse->get(genId.id());
    if (entry.id() >= dense->size()) return nullptr;
    if (entry.gen() != genId.gen()) return nullptr;
    return &dense->get(entry.id());
  }

  T* operator[](GenId genId) {
    return get(genId);
  }

  GenId find(const T& value) {
    for (uint32_t i = 0; i < dense->size(); i++) {
      if (dense->get(i) == value) {
        uint32_t sparse_idx = dense_to_sparse->get(i);
        return GenId::create(sparse_idx, sparse->get(sparse_idx).gen());
      }
    }
    return {UINT32_MAX};
  }

  bool contains(GenId genId) {
    if (genId.id() >= sparse->size()) return false;
    const auto& entry = sparse->get(genId.id());
    if (entry.id() >= dense->size()) return false;
    return entry.gen() == genId.gen();
  }

  void clear() {
    dense->clear();
    dense_to_sparse->clear();

    // Rebuild free list while preserving generations
    for (uint32_t i = 0; i < N-1; i++) {
      auto& entry = sparse->get(i);
      entry.increment_gen();
      entry.set_id(i + 1);
    }

    auto& last = sparse->get(N-1);
    last.increment_gen();
    last.set_id(N);

    free_head = 0;
  }

  uint32_t size() const { return dense->size(); }

  bool empty() const { return dense->empty(); }

  bool is_full() const { return dense->size() == dense->capacity(); }

  bool capacity() const { return dense->capacity(); }

  using Iterator = typename ArrayRT<T>::Iterator;
  Iterator begin() { return dense->begin(); }
  Iterator end() { return dense->end(); }
};

// NOTE: Arena
class Arena {
public:
  uint32_t capacity;
  uint32_t used;
  char* memory;

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;
  Arena(Arena&& other) = delete;
  Arena& operator=(Arena&& other) = delete;

  explicit Arena(uint32_t size) {
    memory = (char*)malloc(size);
    if (!memory) LOG_ASSERT(false, "Failed to allocate memory!");
    memset(memory, 0, size);
    capacity = size;
    used = 0;
  }

  char& get(uint32_t idx) {
    LOG_ASSERT(idx < used, "Index out of bounds!");
    return memory[idx];
  }

  char& operator[](uint32_t idx) {
    return get(idx);
  }

  template<typename T, typename Arg>
  T& create(Arg& arg) {
    T* ptr = alloc_raw<T>();
    return *(new (ptr) T(arg));
  }

  template<typename T>
  T* alloc_raw() { // Guaranteed to return valid memory or assert
    uint32_t size = sizeof(T);
    uint32_t aligned_size = (size + 7) & ~7;  // 8-byte alignment
    if (used + aligned_size > capacity) LOG_ASSERT(false, "Arena is full");
    T* result = (T*)(memory + used);
    used += aligned_size;
    return result;
  }

  template<typename T>
  T& alloc() {
    return *alloc_raw<T>();
  }

  template<typename T>
  T* alloc_raw(uint32_t size) { // Guaranteed to return valid memory or assert
    uint32_t aligned_size = (size + 7) & ~7;  // 8-byte alignment
    if (used + aligned_size > capacity) LOG_ASSERT(false, "Arena is full");
    T* result = (T*)(memory + used);
    used += aligned_size;
    return result;
  }

  template<typename T>
  T& alloc(uint32_t size) {
    return *alloc_raw<T>(size);
  }

  template<typename T>
  T* alloc_count_raw(uint32_t count) { // Guaranteed to return valid memory or assert
    uint32_t total_size = sizeof(T) * count;
    uint32_t aligned_size = (total_size + 7) & ~7;  // 8-byte alignment
    if (used + aligned_size > capacity) LOG_ASSERT(false, "Arena is full");
    T* result = reinterpret_cast<T*>(memory + used);
    used += aligned_size;
    return result;
  }

  template<typename T>
  T& alloc_count(uint32_t count) {
    return *alloc_count_raw<T>(count);
  }

  template <typename E, typename M> 
  E& fetch(const char* key) {
    M* map_ptr = reinterpret_cast<M*>(memory);
    void* ptr = map_ptr->get(key);
    return *reinterpret_cast<E*>(ptr);
  }

  template<typename T>
  ArrayRT<T>& create_array_rt(uint32_t maxElements) {
    uint32_t total_size = sizeof(ArrayRT<T>) + sizeof(T) * (maxElements - 1);
    ArrayRT<T>& arr = alloc<ArrayRT<T>>(total_size);
    arr.init(maxElements);
    return arr;
  }

  template<typename T, uint32_t N>
  ArrayCT<T, N>& create_array_ct() {
    ArrayCT<T, N>& arr = alloc<ArrayCT<T, N>>();
    arr.init();
    return arr;
  }

  template<typename KeyType, typename ValueType>
  MapRT<KeyType, ValueType>& create_map_rt(uint32_t maxElements) {
    MapRT<KeyType, ValueType>& map = alloc<MapRT<KeyType, ValueType>>(sizeof(MapRT<KeyType, ValueType>));
    ArrayRT<Entry<KeyType, ValueType>>& entries = create_array_rt<Entry<KeyType, ValueType>>(maxElements);
    map.init(entries);
    return map;
  }

  template<typename KeyType, typename ValueType, uint32_t N>
  MapCT<KeyType, ValueType, N>& create_map_ct() {
    MapCT<KeyType, ValueType, N>& map = alloc<MapCT<KeyType, ValueType, N>>();
    map.init();
    return map;
  }

  template<typename KeyType, typename ValueType>
  HashMapRT<KeyType, ValueType>& create_hashmap_rt(uint32_t maxElements) {
    HashMapRT<KeyType, ValueType>& map = alloc<HashMapRT<KeyType, ValueType>>();
    ArrayRT<HashEntry<KeyType, ValueType>>& entries = create_array_rt<HashEntry<KeyType, ValueType>>(maxElements);
    map.init(entries, maxElements);
    return map;
  }

  template<typename KeyType, typename ValueType, uint32_t N>
  HashMapCT<KeyType, ValueType, N>& create_hashmap_ct() {
    HashMapCT<KeyType, ValueType, N>& map = alloc<HashMapCT<KeyType, ValueType, N>>();
    map.init();
    return map;
  }

  template<typename T>
  GenSparseSetRT<T>& create_gen_sparse_set_rt(uint32_t maxElements) {
    GenSparseSetRT<T>& genSparseSet = alloc<GenSparseSetRT<T>>();
    ArrayRT<T>& dense = create_array_rt<T>(maxElements);
    ArrayRT<GenId>& sparse = create_array_rt<GenId>(maxElements);
    ArrayRT<uint32_t>& dense_to_sparse = create_array_rt<uint32_t>(maxElements);
    genSparseSet.init(maxElements, dense, sparse, dense_to_sparse);
    return genSparseSet;
  }

  template<typename T, uint32_t N>
  GenSparseSetCT<T, N>& create_gen_sparse_set_ct() {
    GenSparseSetCT<T, N>& genSparseSet = alloc<GenSparseSetCT<T, N>>();
    genSparseSet.init();
    return genSparseSet;
  }

  void clear() {
    used = 0;
    memset(memory, 0, capacity); // Sets the memory to 0
  }

  uint32_t size() const {
    return used;
  }

  uint32_t available() const {
    return capacity - used;
  }

  bool is_empty() const {
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
uint64_t get_timestamp(const char* file);
bool file_exists(const char* filePath);
uint32_t get_file_size(const char* filePath);
char* read_file(const char* filePath, Arena& arena);
void write_file(const char* filePath, const char* buffer, uint32_t size);
bool copy_file(const char* fileName, const char* outputName, Arena& arena);
void remove_file(const char* fileName);

//NOTE: Testing
bool CompareFloat(float a, float b, float epsilon = 0.0001f);
bool CompareIntArrays(const int *a, const int *b, uint32_t size);
bool CompareFloatArrays(const float *a, const float *b, uint32_t size);
bool CompareUCharArrays(const unsigned char *a, const unsigned char *b, uint32_t size);
bool CompareUIntArrays(const unsigned int *a, const unsigned int *b, uint32_t size);
bool CompareUShortArrays(const unsigned short *a, const unsigned short *b, uint32_t size);
