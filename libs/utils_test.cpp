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
  char* failedMsg = "[ FAILED ] create_and_fetch_arena_in_different_scope_CT_test";
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
  char* failedMsg = "[ FAILED ] create_and_fetch_arena_in_different_scope_RT_test";
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

void create_arena_clear_test() {
  Arena& arena = Arena::create(KB(1));
  char* failedMsg = "[ FAILED ] create_arena_clear_test";
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

// NOTE: File I/O
void file_io_test() {
  char* failedMsg = "[ FAILED ] create_and_remove_file_test, please clean up";
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
