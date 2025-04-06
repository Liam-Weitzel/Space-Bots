#include "utils.h"
#include "utils_test.h"

// g++ utils_benchmark.cpp utils.cpp utils_test.cpp -O0 -g -o utils_benchmark

const size_t CACHE_SIZE = MB(1); // 1MB for quick methods
const size_t FULL_CACHE_SIZE = MB(4); // 4MB for thorough method
const size_t STRIDE = 64;
const int ITERATIONS = 100;

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

void run_iterator_tests(CacheClearMethod method) {
  printf("\n=== Iterator Tests (Cache: %s) ===\n",
         method == CacheClearMethod::Thorough ? "Thorough" :
         method == CacheClearMethod::Stride ? "Stride" :
         method == CacheClearMethod::Random ? "Random" : "None");

  auto run_ct_test = [method]() {
    if (method == CacheClearMethod::Thorough) clear_cache_thorough();
    else if (method == CacheClearMethod::Stride) clear_cache_stride();
    else if (method == CacheClearMethod::Random) clear_cache_random();
    iterators_arrays_CT_test();
  };
  printf("  CT Array Iterator Test: ");
  BENCHMARK(run_ct_test, ITERATIONS);

  auto run_rt_test = [method]() {
    if (method == CacheClearMethod::Thorough) clear_cache_thorough();
    else if (method == CacheClearMethod::Stride) clear_cache_stride();
    else if (method == CacheClearMethod::Random) clear_cache_random();
    iterators_arrays_RT_test();
  };
  printf("  RT Array Iterator Test: ");
  BENCHMARK(run_rt_test, ITERATIONS);
}

void run_arena_tests(CacheClearMethod method) {
  printf("\n=== Arena Tests (Cache: %s) ===\n",
         method == CacheClearMethod::Thorough ? "Thorough" :
         method == CacheClearMethod::Stride ? "Stride" :
         method == CacheClearMethod::Random ? "Random" : "None");

  auto run_ct_arena = [method]() {
    if (method == CacheClearMethod::Thorough) clear_cache_thorough();
    else if (method == CacheClearMethod::Stride) clear_cache_stride();
    else if (method == CacheClearMethod::Random) clear_cache_random();
    create_and_fetch_arena_in_different_scope_CT_test();
  };
  printf("  Arena CT Create/Fetch Test: ");
  BENCHMARK(run_ct_arena, ITERATIONS);

  auto run_rt_arena = [method]() {
    if (method == CacheClearMethod::Thorough) clear_cache_thorough();
    else if (method == CacheClearMethod::Stride) clear_cache_stride();
    else if (method == CacheClearMethod::Random) clear_cache_random();
    create_and_fetch_arena_in_different_scope_RT_test();
  };
  printf("  Arena RT Create/Fetch Test: ");
  BENCHMARK(run_rt_arena, ITERATIONS);

  auto run_clear = [method]() {
    if (method == CacheClearMethod::Thorough) clear_cache_thorough();
    else if (method == CacheClearMethod::Stride) clear_cache_stride();
    else if (method == CacheClearMethod::Random) clear_cache_random();
    create_arena_clear_test();
  };
  printf("  Arena Clear Test: ");
  BENCHMARK(run_clear, ITERATIONS);

  auto run_ct_hashmap = [method]() {
    if (method == CacheClearMethod::Thorough) clear_cache_thorough();
    else if (method == CacheClearMethod::Stride) clear_cache_stride();
    else if (method == CacheClearMethod::Random) clear_cache_random();
    create_hashmap_in_arena_CT_test();
  };
  printf("  Arena CT Hashmap Test: ");
  BENCHMARK(run_ct_hashmap, ITERATIONS);

  auto run_rt_hashmap = [method]() {
    if (method == CacheClearMethod::Thorough) clear_cache_thorough();
    else if (method == CacheClearMethod::Stride) clear_cache_stride();
    else if (method == CacheClearMethod::Random) clear_cache_random();
    create_hashmap_in_arena_RT_test();
  };
  printf("  Arena RT Hashmap Test: ");
  BENCHMARK(run_rt_hashmap, ITERATIONS);

  auto run_ct_sparse_set = [method]() {
    if (method == CacheClearMethod::Thorough) clear_cache_thorough();
    else if (method == CacheClearMethod::Stride) clear_cache_stride();
    else if (method == CacheClearMethod::Random) clear_cache_random();
    gen_sparse_set_ct_test();
  };
  printf("  Arena CT Sparse Set Test: ");
  BENCHMARK(run_ct_sparse_set, ITERATIONS);

  auto run_rt_sparse_set = [method]() {
    if (method == CacheClearMethod::Thorough) clear_cache_thorough();
    else if (method == CacheClearMethod::Stride) clear_cache_stride();
    else if (method == CacheClearMethod::Random) clear_cache_random();
    gen_sparse_set_rt_test();
  };
  printf("  Arena RT Sparse Set Test: ");
  BENCHMARK(run_rt_sparse_set, ITERATIONS);
}

void run_sort_tests(CacheClearMethod method) {
  printf("\n=== Sort Tests (Cache: %s) ===\n",
         method == CacheClearMethod::Thorough ? "Thorough" :
         method == CacheClearMethod::Stride ? "Stride" :
         method == CacheClearMethod::Random ? "Random" : "None");

  auto run_sort = [method]() {
    if (method == CacheClearMethod::Thorough) clear_cache_thorough();
    else if (method == CacheClearMethod::Stride) clear_cache_stride();
    else if (method == CacheClearMethod::Random) clear_cache_random();
    quicksort_test();
  };
  printf("  Sort Test: ");
  BENCHMARK(run_sort, ITERATIONS);
}

void run_file_io_tests(CacheClearMethod method) {
  printf("\n=== File I/O Tests (Cache: %s) ===\n",
         method == CacheClearMethod::Thorough ? "Thorough" :
         method == CacheClearMethod::Stride ? "Stride" :
         method == CacheClearMethod::Random ? "Random" : "None");

  auto run_io = [method]() {
    if (method == CacheClearMethod::Thorough) clear_cache_thorough();
    else if (method == CacheClearMethod::Stride) clear_cache_stride();
    else if (method == CacheClearMethod::Random) clear_cache_random();
    file_io_test();
  };
  printf("  File I/O Operations Test: ");
  BENCHMARK(run_io, ITERATIONS);
}

void run_all_tests(CacheClearMethod method) {
  run_iterator_tests(method);
  run_arena_tests(method);
  run_file_io_tests(method);
  run_sort_tests(method);
}

int main(int argc, char *argv[]) {
  printf("Running performance tests with different cache clearing methods...\n");

  run_all_tests(CacheClearMethod::None);
  run_all_tests(CacheClearMethod::Thorough);
  run_all_tests(CacheClearMethod::Stride);
  run_all_tests(CacheClearMethod::Random);

  return 0;
}

