#include "utils.h"
#include "utils_test.h"
#include <chrono>

// g++ utils_performance_test.cpp utils.cpp utils_test.cpp -O0 -g -o utils_performance_test

const int NUM_ITERATIONS = 100;
const size_t CACHE_SIZE = MB(1); // 1MB for quick methods
const size_t FULL_CACHE_SIZE = MB(4); // 4MB for thorough method
const size_t STRIDE = 64;

constexpr size_t THOROUGH_ARRAY_SIZE = FULL_CACHE_SIZE / sizeof(int);
constexpr size_t QUICK_ARRAY_SIZE = CACHE_SIZE / sizeof(int);

enum class CacheClearMethod {
  Thorough,
  Stride,
  Random,
  None
};

void clear_cache_thorough() {
  ArrayCT<int, THOROUGH_ARRAY_SIZE> cache_clearer;
  for (size_t i = 0; i < THOROUGH_ARRAY_SIZE; i++) {
    cache_clearer.add(i);
  }
  volatile int sum = 0;
  for (int i : cache_clearer) {
    sum += i;
  }
}

void clear_cache_stride() {
  static ArrayCT<int, QUICK_ARRAY_SIZE> cache_clearer;
  if (cache_clearer.empty()) {
    for (size_t i = 0; i < QUICK_ARRAY_SIZE; i++) {
      cache_clearer.add(i);
    }
  }
  volatile int sum = 0;
  for (size_t i = 0; i < cache_clearer.size(); i += STRIDE) {
    cache_clearer[i] = i;
    sum += cache_clearer[i];
  }
}

void clear_cache_random() {
  static ArrayCT<int, QUICK_ARRAY_SIZE> cache_clearer;
  if (cache_clearer.empty()) {
    for (size_t i = 0; i < QUICK_ARRAY_SIZE; i++) {
      cache_clearer.add(i);
    }
  }
  volatile int sum = 0;
  for (size_t i = 0; i < 1000; i++) {
    size_t idx = (rand() * STRIDE) % cache_clearer.size();
    cache_clearer[idx] = i;
    sum += cache_clearer[idx];
  }
}

template<typename F>
double measure_average_time(F&& func, const char* operation_name, CacheClearMethod method) {
  using namespace std::chrono;
  using Duration = duration<double, std::nano>;

  auto total = Duration::zero();
  for (int i = 0; i < NUM_ITERATIONS; i++) {
    switch (method) {
      case CacheClearMethod::Thorough: clear_cache_thorough(); break;
      case CacheClearMethod::Stride: clear_cache_stride(); break;
      case CacheClearMethod::Random: clear_cache_random(); break;
      case CacheClearMethod::None: break;
    }

    auto t1 = high_resolution_clock::now();
    func();
    auto t2 = high_resolution_clock::now();
    total += t2 - t1;
  }

  double avg_ns = total.count() / NUM_ITERATIONS;
  printf("        %s avg: %.2f ns\n", operation_name, avg_ns);
  return avg_ns;
}

void run_iterator_tests(CacheClearMethod method) {
  printf("\n    === Iterator Tests (Cache: %s) ===\n",
         method == CacheClearMethod::Thorough ? "Thorough" :
         method == CacheClearMethod::Stride ? "Stride" :
         method == CacheClearMethod::Random ? "Random" : "None");

  measure_average_time(
    []() { iterators_arrays_CT_test(); },
    "CT Array Iterator Test",
    method
  );

  measure_average_time(
    []() { iterators_arrays_RT_test(); },
    "RT Array Iterator Test",
    method
  );
}

void run_arena_tests(CacheClearMethod method) {
  printf("\n    === Arena Tests (Cache: %s) ===\n",
         method == CacheClearMethod::Thorough ? "Thorough" :
         method == CacheClearMethod::Stride ? "Stride" :
         method == CacheClearMethod::Random ? "Random" : "None");

  measure_average_time(
    []() { create_and_fetch_arena_in_different_scope_CT_test(); },
    "Arena CT Create/Fetch Test",
    method
  );

  measure_average_time(
    []() { create_and_fetch_arena_in_different_scope_RT_test(); },
    "Arena RT Create/Fetch Test",
    method
  );

  measure_average_time(
    []() { create_arena_clear_test(); },
    "Arena Clear Test",
    method
  );
}

void run_file_io_tests(CacheClearMethod method) {
  printf("\n    === File I/O Tests (Cache: %s) ===\n",
         method == CacheClearMethod::Thorough ? "Thorough" :
         method == CacheClearMethod::Stride ? "Stride" :
         method == CacheClearMethod::Random ? "Random" : "None");

  measure_average_time(
    []() { file_io_test(); },
    "File I/O Operations Test",
    method
  );
}

void run_all_tests(CacheClearMethod method) {
  run_iterator_tests(method);
  run_arena_tests(method);
  run_file_io_tests(method);
}

int main(int argc, char *argv[]) {
  printf("Running performance tests with different cache clearing methods...\n");

  printf("\n=== No Cache Clearing ===");
  run_all_tests(CacheClearMethod::None);

  printf("\n=== Thorough Cache Clearing ===");
  run_all_tests(CacheClearMethod::Thorough);

  printf("\n=== Stride-based Cache Clearing ===");
  run_all_tests(CacheClearMethod::Stride);

  printf("\n=== Random Cache Clearing ===");
  run_all_tests(CacheClearMethod::Random);

  return 0;
}
