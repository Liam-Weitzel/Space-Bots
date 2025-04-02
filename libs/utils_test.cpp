#include "utils_test.h"
#include "utils.h"
#include <cstdlib>
#include <cstring>

// NOTE: Datastructures
void iterators_arrays_CT_test() {
  // Create CT array on stack
  ArrayCT<Entity, 3> array = {};

  // Create an array of entities
  Entity entities[] = {
    Entity{1, "Entity 1"},
    Entity{2, "Entity 2"},
    Entity{3, "Entity 3"}
  };

  // Add entities to array
  array.add(entities, 3);

  // Using int i here as I need to test the iterator...
  int i = 0;
  for(Entity entity : entities) {
    LOG_ASSERT(entities[i] == entity, "[ FAILED ] iterators_arrays_CT_test");
    i++;
  }
  LOG_TRACE("[ PASSED ] iterators_arrays_CT_test")
}

void iterators_arrays_RT_test() {
  // Create RT array on heap
  size_t total_size = sizeof(ArrayRT<Entity>) + sizeof(Entity) * (3 - 1);
  ArrayRT<Entity>* array = (ArrayRT<Entity>*)malloc(total_size);
  array->capacity = 3;
  array->count = 0;

  // Create an array of entities
  Entity entities[] = {
    Entity{1, "Entity 1"},
    Entity{2, "Entity 2"},
    Entity{3, "Entity 3"}
  };

  // Add entities to array
  array->add(entities, 3);

  // Using int i here as I need to test the iterator...
  int i = 0;
  for(Entity entity : entities) {
    LOG_ASSERT(entities[i] == entity, "[ FAILED ] iterators_arrays_RT_test");
    i++;
  }

  free(array);
  LOG_TRACE("[ PASSED ] iterators_arrays_RT_test")
}

void create_and_fetch_arena_in_different_scope_CT_test() {
  const char* failedMsg = "[ FAILED ] create_and_fetch_arena_in_different_scope_CT_test";
  Arena& arena = Arena::create(80);
  MapCT<const char*, void*, 1>& arenaIndex = arena.create_map_ct<const char*, void*, 1>();
  {
    ArrayCT<Entity, 3>& entitiesArray = arena.create_array_ct<Entity, 3>();
    arenaIndex["entities"] = &entitiesArray;

    // Create an array of entities
    Entity entities[] = {
      Entity{1, "Entity 1"},
      Entity{2, "Entity 2"},
      Entity{3, "Entity 3"}
    };

    // Add entities to the array within the arena
    entitiesArray.add(entities, 3);
  }
  {
    // Access entities from the array from scratch
    ArrayCT<Entity, 3>& entitiesFetched = arena.fetch<ArrayCT<Entity, 3>, MapCT<const char*, void*, 1>>("entities");

    // Access entities
    Entity e0 = entitiesFetched[0];
    Entity e1 = entitiesFetched[1];
    Entity e2 = entitiesFetched[2];

    LOG_ASSERT(e0.id == 1 && strcmp(e0.name, "Entity 1") == 0 &&
               e1.id == 2 && strcmp(e1.name, "Entity 2") == 0 &&
               e2.id == 3 && strcmp(e2.name, "Entity 3") == 0
               , failedMsg);
  }
  delete &arena;
  LOG_TRACE("[ PASSED ] create_and_fetch_arena_in_different_scope_CT_test")
}

void create_and_fetch_arena_in_different_scope_RT_test() {
  const char* failedMsg = "[ FAILED ] create_and_fetch_arena_in_different_scope_RT_test";
  Arena& arena = Arena::create(88); //NOTE: 8 more bytes than CT... 2x extra 32 bit int for size...
  MapRT<const char*, void*>& arenaIndex = arena.create_map_rt<const char*, void*>(1);
  {
    ArrayRT<Entity>& entitiesArray = arena.create_array_rt<Entity>(3);
    arenaIndex["entities"] = &entitiesArray;

    // Create an array of entities
    Entity entities[] = {
      Entity{1, "Entity 1"},
      Entity{2, "Entity 2"},
      Entity{3, "Entity 3"}
    };

    // Add entities to the array within the arena
    entitiesArray.add(entities, 3);
  }
  {
    // Access entities from the array from scratch
    ArrayRT<Entity>& entitiesFetched = arena.fetch<ArrayRT<Entity>, MapRT<const char*, void*>>("entities");

    // Access entities
    Entity e0 = entitiesFetched[0];
    Entity e1 = entitiesFetched[1];
    Entity e2 = entitiesFetched[2];

    LOG_ASSERT(e0.id == 1 && strcmp(e0.name, "Entity 1") == 0 &&
               e1.id == 2 && strcmp(e1.name, "Entity 2") == 0 &&
               e2.id == 3 && strcmp(e2.name, "Entity 3") == 0
               , failedMsg);
  }
  delete &arena;
  LOG_TRACE("[ PASSED ] create_and_fetch_arena_in_different_scope_RT_test")
}

void create_hashmap_in_arena_CT_test() {
  const char* failedMsg = "[ FAILED ] create_hashmap_in_arena_CT_test";
  Arena& arena = create_arena(KB(4));
  auto& arena_index = arena.create_hashmap_ct<const char*, void*, 1>();

  // Create hashmap and test basic operations
  auto& map = arena.create_hashmap_ct<const char*, int, 16>();
  arena_index["map"] = &map;

  // Test empty state
  LOG_ASSERT(map.empty() && map.size() == 0, failedMsg);

  // Test insertion
  map["test1"] = 42;
  map["test2"] = 84;

  LOG_ASSERT(map.size() == 2, failedMsg);

  auto& map_fetched = arena.fetch<HashMapCT<const char*, int, 16>, HashMapCT<const char*, void*, 1>>("map");
  LOG_ASSERT(map_fetched["test1"] == 42, failedMsg);

  // Test retrieval
  LOG_ASSERT(map["test1"] == 42 && map["test2"] == 84, failedMsg);

  // Test contains
  LOG_ASSERT(map.contains("test1") && map.contains("test2"), failedMsg);
  LOG_ASSERT(!map.contains("nonexistent"), failedMsg);

  // Test removal
  map.remove("test1");
  LOG_ASSERT(!map.contains("test1") && map.size() == 1, failedMsg);

  // Test clear
  map.clear();
  LOG_ASSERT(map.empty() && map.size() == 0, failedMsg);

  // Test load factor limit
  for(int i = 0; i < 11; i++) { // Should work up to 70% capacity
      char key[8];
      sprintf(key, "key%d", i);
      map[key] = i;
  }

  LOG_ASSERT(map.size() == 11, failedMsg);
  LOG_TRACE("[ PASSED ] create_hashmap_in_arena_CT_test")
}

void create_hashmap_in_arena_RT_test() {
  const char* failedMsg = "[ FAILED ] create_hashmap_in_arena_CT_test";
  Arena& arena = create_arena(KB(4));
  auto& arena_index = arena.create_hashmap_rt<const char*, void*>(1);

  // Create hashmap and test basic operations
  auto& map = arena.create_hashmap_rt<const char*, int>(16);
  arena_index["map"] = &map;

  // Test empty state
  LOG_ASSERT(map.empty() && map.size() == 0, failedMsg);

  // Test insertion
  map["test1"] = 42;
  map["test2"] = 84;

  LOG_ASSERT(map.size() == 2, failedMsg);

  auto& map_fetched = arena.fetch<HashMapRT<const char*, int>, HashMapRT<const char*, void*>>("map");
  LOG_ASSERT(map_fetched["test1"] == 42, failedMsg);

  // Test retrieval
  LOG_ASSERT(map["test1"] == 42 && map["test2"] == 84, failedMsg);

  // Test contains
  LOG_ASSERT(map.contains("test1") && map.contains("test2"), failedMsg);
  LOG_ASSERT(!map.contains("nonexistent"), failedMsg);

  // Test removal
  map.remove("test1");
  LOG_ASSERT(!map.contains("test1") && map.size() == 1, failedMsg);

  // Test clear
  map.clear();
  LOG_ASSERT(map.empty() && map.size() == 0, failedMsg);
  LOG_TRACE("[ PASSED ] create_hashmap_in_arena_RT_test")
}

void quicksort_test() {
  const char* failedMsg = "[ FAILED ] quicksort_test";
  
  // Test CT array with integers
  {
    ArrayCT<int, 5> arr = {};
    int numbers[] = {64, 34, 25, 12, 22};
    arr.add(numbers, 5);
    
    quicksort(arr);
    
    LOG_ASSERT(arr[0] == 12 && arr[1] == 22 && arr[2] == 25 && 
               arr[3] == 34 && arr[4] == 64, failedMsg);
  }
  
  // Test RT array with integers
  {
    // Calculate total size including the flexible array member
    size_t total_size = offsetof(ArrayRT<int>, elements) + sizeof(int) * 5;
    
    // Allocate memory
    ArrayRT<int>* arr = (ArrayRT<int>*)malloc(total_size);
    
    // Initialize members
    arr->capacity = 5;
    arr->count = 0;
    
    int numbers[] = {64, 34, 25, 12, 22};
    arr->add(numbers, 5);
    
    quicksort(*arr);
    
    LOG_ASSERT((*arr)[0] == 12 && (*arr)[1] == 22 && (*arr)[2] == 25 && 
               (*arr)[3] == 34 && (*arr)[4] == 64, failedMsg);
    
    free(arr);
  }
  
  // Test partial range sorting
  {
    ArrayCT<int, 5> arr = {};
    int numbers[] = {64, 34, 25, 12, 22};
    arr.add(numbers, 5);
    
    quicksort(arr, 1, 3); // Sort only indices 1 through 3
    
    LOG_ASSERT(arr[0] == 64 && arr[1] == 12 && arr[2] == 25 && 
               arr[3] == 34 && arr[4] == 22, failedMsg);
  }
  
  // Test edge cases
  {
    // Empty array
    ArrayCT<int, 1> empty_arr = {};
    quicksort(empty_arr);
    LOG_ASSERT(empty_arr.count == 0, failedMsg);
    
    // Single element
    ArrayCT<int, 1> single_arr = {};
    single_arr.add(42);
    quicksort(single_arr);
    LOG_ASSERT(single_arr[0] == 42, failedMsg);
    
    // Already sorted array
    ArrayCT<int, 3> sorted_arr = {};
    int sorted[] = {1, 2, 3};
    sorted_arr.add(sorted, 3);
    quicksort(sorted_arr);
    LOG_ASSERT(sorted_arr[0] == 1 && sorted_arr[1] == 2 && sorted_arr[2] == 3, failedMsg);
  }
  
  LOG_TRACE("[ PASSED ] quicksort_test");
}

void create_arena_clear_test() {
  Arena& arena = Arena::create(KB(1));
  const char* failedMsg = "[ FAILED ] create_arena_clear_test";
  {
    MapCT<const char*, void*, 1>& arenaIndex = arena.create_map_ct<const char*, void*, 1>();

    ArrayCT<Entity, 3>& entitiesArray = arena.create_array_ct<Entity, 3>();
    arenaIndex["entities"] = &entitiesArray;

    // Create an array of entities
    Entity entities[] = {
      Entity{1, "Entity 1"},
      Entity{2, "Entity 2"},
      Entity{3, "Entity 3"}
    };

    // Add entities to the array within the arena
    entitiesArray.add(entities, 3);

    LOG_ASSERT(arena.size() == 80, failedMsg)
  }
  arena.clear();
  LOG_ASSERT(arena.size() == 0, failedMsg)
  {
    MapCT<const char*, void*, 1>& arenaIndex = arena.create_map_ct<const char*, void*, 1>();

    ArrayCT<Entity, 3>& entitiesArray = arena.create_array_ct<Entity, 3>();
    arenaIndex["entities"] = &entitiesArray;

    // Create an array of entities
    Entity entities[] = {
      Entity{1, "Entity 1"},
      Entity{2, "Entity 2"},
      Entity{3, "Entity 3"}
    };

    // Add entities to the array within the arena
    entitiesArray.add(entities, 3);

    LOG_ASSERT(arena.size() == 80, failedMsg);
  }
  delete &arena;
  LOG_TRACE("[ PASSED ] create_arena_clear_test")
}

void gen_sparse_set_ct_test() {
  Arena& arena = Arena::create(KB(1));
  const char* failedMsg = "[ FAILED ] gen_sparse_set_test";
  
  auto& set = arena.create_gen_sparse_set_ct<Entity, 8>();

  // Test initial state
  LOG_ASSERT(set.empty(), failedMsg);
  LOG_ASSERT(set.size() == 0, failedMsg);

  // Test invalid lookups
  GenId invalid_id = GenId::create(9999, 0);  // Out of bounds
  LOG_ASSERT(!set.contains(invalid_id), failedMsg);
  LOG_ASSERT(set.get(invalid_id) == nullptr, failedMsg);

  // Add and test first entity
  GenId id1 = set.add(Entity{1, "First"});
  LOG_ASSERT(set.size() == 1, failedMsg);
  LOG_ASSERT(set.contains(id1), failedMsg);
  LOG_ASSERT(set.get(id1)->id == 1, failedMsg);
  
  // Fill the set
  GenId ids[7];
  for(int i = 0; i < 7; i++) {
    ids[i] = set.add(Entity{i + 2, "Entity"});
    LOG_ASSERT(set.contains(ids[i]), failedMsg);
  }
  LOG_ASSERT(set.size() == 8, failedMsg);

  // Test removal and generations
  set.remove(ids[2]);  // Remove middle element
  LOG_ASSERT(set.size() == 7, failedMsg);
  LOG_ASSERT(!set.contains(ids[2]), failedMsg);
  LOG_ASSERT(set.get(ids[2]) == nullptr, failedMsg);

  // Test reuse of slot with generation increment
  GenId new_id = set.add(Entity{99, "Reused"});
  LOG_ASSERT(new_id.id() == ids[2].id(), failedMsg);
  LOG_ASSERT(new_id.gen() == ids[2].gen() + 1, failedMsg);
  LOG_ASSERT(set.contains(new_id), failedMsg);
  LOG_ASSERT(!set.contains(ids[2]), failedMsg);
  LOG_ASSERT(set.get(new_id)->id == 99, failedMsg);

  // Test multiple removes and adds
  set.remove(ids[0]);
  set.remove(ids[4]);
  set.remove(new_id);
  LOG_ASSERT(set.size() == 5, failedMsg);

  GenId reused_ids[3];
  for(int i = 0; i < 3; i++) {
    reused_ids[i] = set.add(Entity{100 + i, "Reused"});
    LOG_ASSERT(set.contains(reused_ids[i]), failedMsg);
    LOG_ASSERT(set.get(reused_ids[i])->id == 100 + i, failedMsg);
  }
  LOG_ASSERT(set.size() == 8, failedMsg);

  // Test clear and reuse
  set.clear();
  LOG_ASSERT(set.empty(), failedMsg);
  LOG_ASSERT(set.size() == 0, failedMsg);

  // Test that generations persisted after clear
  GenId post_clear_id = set.add(Entity{1, "Post Clear"});
  // Should have incremented generation for previously used slot
  LOG_ASSERT(post_clear_id.gen() > 0, failedMsg);
  
  // Verify old IDs are invalid
  LOG_ASSERT(!set.contains(id1), failedMsg);
  LOG_ASSERT(!set.contains(new_id), failedMsg);
  for(auto& id : ids) {
    LOG_ASSERT(!set.contains(id), failedMsg);
  }
  for(auto& id : reused_ids) {
    LOG_ASSERT(!set.contains(id), failedMsg);
  }

  // Test iterator & find
  int count = 0;
  for(const auto& entity : set) {
    LOG_ASSERT(entity.id == 1 || (entity.id >= 100 && entity.id <= 102), failedMsg);
    GenId* found = set.find(entity);
    LOG_ASSERT(found != nullptr, failedMsg);
    LOG_ASSERT(set.get(*found)->id == entity.id, failedMsg);
    count++;
  }
  LOG_ASSERT(count == 1, failedMsg); // Should only have the post-clear entity

  delete &arena;
  LOG_TRACE("[ PASSED ] gen_sparse_set_test");
}

// NOTE: File I/O
void file_io_test() {
  const char* failedMsg = "[ FAILED ] create_and_remove_file_test, please clean up";
  const char* filePath = "./create_and_remove_file_test";
  const char* contents = "create_and_remove_file_test";
  Arena& arena = Arena::create(KB(1));

  write_file(filePath, contents, strlen(contents));
  LOG_ASSERT(file_exists(filePath), failedMsg);

  const char* filePathCopy = "./create_and_remove_file_test_copy";
  copy_file(filePath, filePathCopy, arena);
  LOG_ASSERT(file_exists(filePathCopy), failedMsg);

  LOG_ASSERT(get_file_size(filePath) == strlen(contents), failedMsg);
  LOG_ASSERT(get_file_size(filePathCopy) == strlen(contents), failedMsg);

  char* read_contents = read_file(filePath, arena);
  LOG_ASSERT(strcmp(read_contents, contents) == 0, failedMsg);
  read_contents = read_file(filePathCopy, arena);
  LOG_ASSERT(strcmp(read_contents, contents) == 0, failedMsg);

  long long timestamp = get_timestamp(filePath);
  LOG_ASSERT(timestamp > 0, failedMsg);
  timestamp = get_timestamp(filePathCopy);
  LOG_ASSERT(timestamp > 0, failedMsg);

  remove_file(filePathCopy);
  LOG_ASSERT(!file_exists(filePathCopy), failedMsg);
  remove_file(filePath);
  LOG_ASSERT(!file_exists(filePath), failedMsg);

  delete &arena;
  LOG_TRACE("[ PASSED ] create_and_remove_file_test");
}
