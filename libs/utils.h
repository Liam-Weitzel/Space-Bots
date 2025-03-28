#pragma once

#include <cstddef>
#include <cstdint>
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
      count += amount;
  }

  void init(int amount) noexcept {
      reserve(amount);
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
      LOG_ASSERT(count + amount <= capacity, "Cannot reserve more than capacity!");
      count += amount;
  }

  void init(int amount) noexcept {
      reserve(amount);
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

  void set_entries(ArrayRT<Entry<KeyType, ValueType>>* e) noexcept {
    entries = e;
  }

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

  void init(HashEntry<KeyType, ValueType>* e, int cap) noexcept {
    entries = e;
    capacity = cap;
    clear();
  }

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
struct Arena {
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
    arr.count = 0;
    return arr;
  }

  template<typename T, int N>
  ArrayCT<T, N>& create_array_ct() {
    ArrayCT<T, N>& arr = alloc<ArrayCT<T, N>>();
    arr.count = 0;
    return arr;
  }

  template<typename KeyType, typename ValueType>
  MapRT<KeyType, ValueType>& create_map_rt(size_t capacity) {
    MapRT<KeyType, ValueType>& map = alloc<MapRT<KeyType, ValueType>>(sizeof(MapRT<KeyType, ValueType>));
    ArrayRT<Entry<KeyType, ValueType>>& entries = create_array_rt<Entry<KeyType, ValueType>>(capacity);
    map.set_entries(&entries);
    return map;
  }

  template<typename KeyType, typename ValueType, int N>
  MapCT<KeyType, ValueType, N>& create_map_ct() {
    MapCT<KeyType, ValueType, N>& map = alloc<MapCT<KeyType, ValueType, N>>();
    return map;
  }

  template<typename KeyType, typename ValueType>
  HashMapRT<KeyType, ValueType>& create_hashmap_rt(size_t capacity) {
    HashMapRT<KeyType, ValueType>& map = alloc<HashMapRT<KeyType, ValueType>>();
    HashEntry<KeyType, ValueType>* entries = alloc_count_raw<HashEntry<KeyType, ValueType>>(capacity);
    map.init(entries, capacity);
    return map;
  }

  template<typename KeyType, typename ValueType, int N>
  HashMapCT<KeyType, ValueType, N>& create_hashmap_ct() {
    return alloc<HashMapCT<KeyType, ValueType, N>>();
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

// NOTE: ECS

using EntityID = uint32_t;
using ComponentID = uint32_t;
inline ComponentID next_component_id = 0;

template<typename T>
ComponentID get_component_id() {
    static ComponentID id = ++next_component_id;
    return id;
}

template<typename T, int MaxEntities>
struct ComponentPool {
  ArrayCT<T, MaxEntities>* components;
  ArrayCT<EntityID, MaxEntities>* entity_to_index;    // Sparse array
  ArrayCT<EntityID, MaxEntities>* index_to_entity;    // Dense array

  ComponentPool(const ComponentPool&) = delete;
  ComponentPool& operator=(const ComponentPool&) = delete;
  ComponentPool(ComponentPool&& other) = delete;
  ComponentPool& operator=(ComponentPool&& other) = delete;

  void init(ArrayCT<T, MaxEntities>* c, 
            ArrayCT<EntityID, MaxEntities>* sparse, 
            ArrayCT<EntityID, MaxEntities>* dense) noexcept {
    components = c;
    entity_to_index = sparse;
    index_to_entity = dense;
  }

  bool has(EntityID entity) const noexcept {
    return entity < MaxEntities && 
           entity_to_index->get(entity) < index_to_entity->size() &&
           index_to_entity->get(entity_to_index->get(entity)) == entity;
  }

  T& get(EntityID entity) noexcept {
    LOG_ASSERT(has(entity), "Entity doesn't have this component!");
    return components->get(entity_to_index->get(entity));
  }

  void add(EntityID entity, const T& component) noexcept {
    LOG_ASSERT(!has(entity), "Entity already has this component!");
    
    size_t new_idx = index_to_entity->size();
    entity_to_index->get(entity) = new_idx;
    index_to_entity->add(entity);
    components->add(component);
  }

  void remove(EntityID entity) noexcept {
    if (!has(entity)) return;

    size_t idx_to_remove = entity_to_index->get(entity);
    size_t last_idx = index_to_entity->size() - 1;
    EntityID last_entity = index_to_entity->get(last_idx);

    components->get(idx_to_remove) = components->get(last_idx);
    entity_to_index->get(last_entity) = idx_to_remove;
    index_to_entity->get(idx_to_remove) = last_entity;

    components->count--;
    index_to_entity->count--;
  }

  void clear() noexcept {
    components->clear();
    index_to_entity->clear();
    for (size_t i = 0; i < MaxEntities; i++) {
      entity_to_index->get(i) = -1;
    }
  }

  size_t size() const noexcept {
    return index_to_entity->size();
  }
};

template<size_t N, typename... Ts>
struct nth_type;

template<typename First, typename... Rest>
struct nth_type<0, First, Rest...> {
  using type = First;
};

template<size_t N, typename First, typename... Rest>
struct nth_type<N, First, Rest...> {
  using type = typename nth_type<N-1, Rest...>::type;
};

template<typename... Components>
struct ComponentRefs {
  using PoolArray = ArrayCT<void*, sizeof...(Components)>;
  PoolArray refs;
  
  template<typename... Ts>
  ComponentRefs(Ts&... components) {
    void* ptr_array[] = {&components...};
    refs.add(ptr_array, sizeof...(Components));
  }

  template<size_t I>
  auto& get() {
    return *static_cast<typename nth_type<I, Components...>::type*>(refs[I]);
  }
};

template<typename... Components>
struct std::tuple_size<ComponentRefs<Components...>> {
  static constexpr size_t value = sizeof...(Components);
};

template<size_t I, typename... Components>
struct std::tuple_element<I, ComponentRefs<Components...>> {
  using type = typename nth_type<I, Components...>::type&;
};

template<size_t I, typename... Components>
auto& get(ComponentRefs<Components...>& refs) {
  return refs.template get<I>();
}

template<size_t...> struct index_sequence {};

template<size_t N, size_t... I>
struct make_index_sequence_impl : make_index_sequence_impl<N-1, N-1, I...> {};

template<size_t... I>
struct make_index_sequence_impl<0, I...> {
  using type = index_sequence<I...>;
};

template<size_t N>
using make_index_sequence = typename make_index_sequence_impl<N>::type;

template<int MaxEntities, typename... Components>
struct ViewIterator {
  using PoolArray = ArrayCT<void*, sizeof...(Components)>;
  
  size_t current_index;
  ArrayCT<EntityID, MaxEntities>* dense_array;
  PoolArray* pools;

  ViewIterator(size_t start_index,
               ArrayCT<EntityID, MaxEntities>* dense,
               PoolArray* pools_array) 
               : current_index(start_index)
               , dense_array(dense)
               , pools(pools_array) {
  }

  bool operator!=(const ViewIterator& other) const {
    return current_index != other.current_index;
  }

  ViewIterator& operator++() {
    do {
      ++current_index;
    } while(current_index < dense_array->size() && 
            !has_all_components(dense_array->get(current_index)));
    return *this;
  }

  ComponentRefs<Components...> operator*() {
    EntityID entity = dense_array->get(current_index);
    return make_refs(entity, make_index_sequence<sizeof...(Components)>{});
  }

  bool has_all_components(EntityID entity) {
    return check_components(entity, make_index_sequence<sizeof...(Components)>{});
  }

  template<size_t... I>
  bool check_components(EntityID entity, index_sequence<I...>) {
    return (static_cast<ComponentPool<Components, MaxEntities>*>(pools->get(I))->has(entity) && ...);
  }

  template<size_t... I>
  ComponentRefs<Components...> make_refs(EntityID entity, index_sequence<I...>) {
    return ComponentRefs<Components...>(
      (static_cast<ComponentPool<Components, MaxEntities>*>(pools->get(I)))->get(entity)...
    );
  }
};

template<int MaxEntities, typename... Components>
class View {
  using PoolArray = ArrayCT<void*, sizeof...(Components)>;
  
  ArrayCT<EntityID, MaxEntities>* dense_arrays[sizeof...(Components)];
  PoolArray pools;
  ArrayCT<EntityID, MaxEntities>* smallest_dense_array;
  size_t smallest_size;

public:
  View(ComponentPool<Components, MaxEntities>*... pools_array) {
    // Store pools for component access
    void* ptr_array[] = {pools_array...};
    pools.add(ptr_array, sizeof...(Components));
    
    // Store dense arrays and find smallest
    store_dense_arrays(pools_array...);
  }

private:
  template<typename First, typename... Rest>
  void store_dense_arrays(ComponentPool<First, MaxEntities>* first, 
                          ComponentPool<Rest, MaxEntities>*... rest) {
    static int index = 0;
    dense_arrays[index] = first->index_to_entity;
    
    // Initialize smallest on first call
    if (index == 0) {
      smallest_size = first->index_to_entity->size();
      smallest_dense_array = first->index_to_entity;
    }
    // Update smallest if current is smaller
    else if (first->index_to_entity->size() < smallest_size) {
      smallest_size = first->index_to_entity->size();
      smallest_dense_array = first->index_to_entity;
    }
    
    index++;
    
    if constexpr(sizeof...(Rest) > 0) {
      store_dense_arrays(rest...);
    } else {
      index = 0; // Reset for next use
    }
  }

public:
  ViewIterator<MaxEntities, Components...> begin() {
    size_t start = 0;
    while(start < smallest_dense_array->size() && 
          !ViewIterator<MaxEntities, Components...>(start, smallest_dense_array, &pools)
          .has_all_components(smallest_dense_array->get(start))) {
      ++start;
    }
    return ViewIterator<MaxEntities, Components...>(start, smallest_dense_array, &pools);
  }

  ViewIterator<MaxEntities, Components...> end() {
    return ViewIterator<MaxEntities, Components...>(
      smallest_dense_array->size(),
      smallest_dense_array,
      &pools
    );
  }
};

template<int MaxEntities, int MaxComponents = 32>
struct ECS {
  EntityID next_entity;
  Arena& arena;
  void** component_pools;
  size_t component_count;

  ECS(const ECS&) = delete;
  ECS& operator=(const ECS&) = delete;
  ECS(ECS&& other) = delete;
  ECS& operator=(ECS&& other) = delete;

  explicit ECS(Arena& arena_ref) : arena(arena_ref) {
    next_entity = 0;
    component_pools = arena.alloc_count_raw<void*>(MaxComponents);
    component_count = 0;
  }

  EntityID create_entity() noexcept {
    LOG_ASSERT(next_entity < MaxEntities, "Too many entities!");
    return next_entity++;
  }

  template<typename T>
  ComponentPool<T, MaxEntities>& register_component() noexcept {
    LOG_ASSERT(component_count < MaxComponents, "Too many component types!");
    
    ComponentID id = get_component_id<T>();
    LOG_ASSERT(id < MaxComponents, "Component ID out of bounds!");

    auto& pool = arena.alloc<ComponentPool<T, MaxEntities>>();
    
    // Create the arrays using ArrayCT
    auto& components = arena.create_array_ct<T, MaxEntities>();
    auto& entity_to_index = arena.create_array_ct<EntityID, MaxEntities>();
    auto& index_to_entity = arena.create_array_ct<EntityID, MaxEntities>();

    // Initialize all sparse indices to -1
    entity_to_index.init(MaxEntities);
    for (size_t i = 0; i < MaxEntities; i++) {
      // could be optimized by using signed int & memsetting
      entity_to_index[i] = -1;
    }

    pool.init(&components, &entity_to_index, &index_to_entity);
    component_pools[id] = &pool;
    component_count++;
    
    return pool;
  }

  template<typename T>
  ComponentPool<T, MaxEntities>& get_components() noexcept {
    ComponentID id = get_component_id<T>();
    LOG_ASSERT(component_pools[id] != nullptr, "Component type not registered!");
    return *static_cast<ComponentPool<T, MaxEntities>*>(component_pools[id]);
  }

  template<typename T>
  T& add_component(EntityID entity, const T& component) noexcept {
    auto& pool = get_components<T>();
    pool.add(entity, component);
    return pool.get(entity);
  }

  template<typename T>
  T& get_component(EntityID entity) noexcept {
    return get_components<T>().get(entity);
  }

  template<typename T>
  void remove_component(EntityID entity) noexcept {
    get_components<T>().remove(entity);
  }

  template<typename T>
  bool has_component(EntityID entity) const noexcept {
    ComponentID id = get_component_id<T>();
    if (component_pools[id] == nullptr) return false;
    return static_cast<ComponentPool<T, MaxEntities>*>(component_pools[id])->has(entity);
  }

  template<typename... Components>
  View<MaxEntities, Components...> view() {
    return View<MaxEntities, Components...>(
      &get_components<Components>()...
    );
  }
};

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
