#include "utils_test.h"
#include "utils.h"

void arena_create_and_fetch_in_different_scope_test() {
  Arena* arena = new Arena(80);
  MapCT<char*, void*, 1>* arenaIndex = arena->create_map_ct<char*, void*, 1>();
  {
    ArrayCT<Entity, 3>* entitiesArray = arena->create_array_ct<Entity, 3>();
    (*arenaIndex)["entities"] = entitiesArray;

    // Create an array of entities
    Entity entities[] = {
      Entity{1, "Entity 1"},
      Entity{2, "Entity 2"},
      Entity{3, "Entity 3"}
    };

    // Add entities to the array within the arena
    entitiesArray->add(entities, 3);
  }
  {
    // Access entities from the array from scratch
    ArrayCT<Entity, 3>* entitiesFetched = arena->fetch<ArrayCT<Entity, 3>, MapCT<char*, void*, 1>>("entities");

    // Access entities
    Entity& e0 = (*entitiesFetched)[0];
    Entity& e1 = (*entitiesFetched)[1];
    Entity& e2 = (*entitiesFetched)[2];

    LOG_ASSERT(e0.id == 1 && e0.name == "Entity 1" &&
               e1.id == 2 && e1.name == "Entity 2" &&
               e2.id == 3 && e2.name == "Entity 3"
               , "test");
  }
}

void arena_e2e_test() {
  { //NOTE: ARENA COMPILE TIME TESTING ZONE
    Arena* arena = new Arena(KB(1));
    MapCT<char*, void*, 3>* arenaIndex = arena->create_map_ct<char*, void*, 3>();

    ArrayCT<Entity, 6>* entitiesArray = arena->create_array_ct<Entity, 6>();
    (*arenaIndex)["entities"] = entitiesArray;

    // Create an array of entities
    Entity entities[] = {
      Entity{1, "Entity 1"},
      Entity{2, "Entity 2"},
      Entity{3, "Entity 3"}
    };

    // Add entities to the array within the arena
    entitiesArray->add(entities, 3);

    LOG_TRACE("Accessing index which doesn't exist: %i", (*arenaIndex)["test"]);

    // Access entities from the array
    Entity& e = (*entitiesArray)[1]; // Get the second entity (id = 2)
    LOG_TRACE("Entity ID: %i, Name: %s", e.id, e.name)

    Entity& e2 = (*entitiesArray)[2]; // Get the third entity (id = 3)
    LOG_TRACE("Entity ID: %i, Name: %s", e2.id, e2.name)

    // Access entities from the array from scratch
    ArrayCT<Entity, 6>* entitiesFetched = arena->fetch<ArrayCT<Entity, 6>, MapCT<char*, void*, 3>>("entities");

    // Access entities
    Entity& eA = (*entitiesFetched)[1]; // Get the second entity (id = 2)
    LOG_TRACE("Entity ID: %i, Name: %s", eA.id, eA.name);

    Entity& eA2 = (*entitiesFetched)[2]; // Get the third entity (id = 3)
    LOG_TRACE("Entity ID: %i, Name: %s", eA2.id, eA2.name);

    ArrayCT<int, 128>* GPUStuffs = arena->create_array_ct<int, 128>();
    (*arenaIndex)["GPUStuffs"] = GPUStuffs;
    GPUStuffs->add(1);
    GPUStuffs->add(1);
    GPUStuffs->add(1);
    GPUStuffs->add(1);
    GPUStuffs->add(1);
    GPUStuffs->add(1);
    ArrayCT<int, 128>* GPUStuffsFetched = arena->fetch<ArrayCT<int, 128>, MapCT<char*, void*, 3>>("GPUStuffs");
    LOG_TRACE("GPUStuffs: %i %i %i %i", (*GPUStuffsFetched)[0], (*GPUStuffsFetched)[1], (*GPUStuffsFetched)[2], (*GPUStuffsFetched)[3]);

    for (Entry<char*, void*> i : *arenaIndex) {
      LOG_TRACE("Key: %s", i.key);
    }

    delete arena;
  }
  { //NOTE: ARENA RUN TIME TESTING ZONE
    Arena* arena = new Arena(KB(1));
    MapRT<char*, void*>* arenaIndex = arena->create_map_rt<char*, void*>(3);

    ArrayRT<Entity>* entitiesArray = arena->create_array_rt<Entity>(6);
    (*arenaIndex)["entities"] = entitiesArray;

    // Create an array of entities
    Entity entities[] = {
      Entity{1, "Entity 1"},
      Entity{2, "Entity 2"},
      Entity{3, "Entity 3"}
    };

    // Add entities to the array within the arena
    entitiesArray->add(entities, 3);

    LOG_TRACE("Accessing index which doesn't exist: %i", (*arenaIndex)["test"]);

    // Access entities from the array
    Entity& e = (*entitiesArray)[1]; // Get the second entity (id = 2)
    LOG_TRACE("Entity ID: %i, Name: %s", e.id, e.name)

    Entity& e2 = (*entitiesArray)[2]; // Get the third entity (id = 3)
    LOG_TRACE("Entity ID: %i, Name: %s", e2.id, e2.name)

    // Access entities from the array from scratch
    ArrayRT<Entity>* entitiesFetched = arena->fetch<ArrayRT<Entity>, MapRT<char*, void*>>("entities");

    // Access entities
    Entity& eA = (*entitiesFetched)[1]; // Get the second entity (id = 2)
    LOG_TRACE("Entity ID: %i, Name: %s", eA.id, eA.name);

    Entity& eA2 = (*entitiesFetched)[2]; // Get the third entity (id = 3)
    LOG_TRACE("Entity ID: %i, Name: %s", eA2.id, eA2.name);

    ArrayRT<int>* GPUStuffs = arena->create_array_rt<int>(128);
    (*arenaIndex)["GPUStuffs"] = GPUStuffs;
    GPUStuffs->add(1);
    GPUStuffs->add(1);
    GPUStuffs->add(1);
    GPUStuffs->add(1);
    GPUStuffs->add(1);
    GPUStuffs->add(1);
    ArrayRT<int>* GPUStuffsFetched = arena->fetch<ArrayRT<int>, MapRT<char*, void*>>("GPUStuffs");
    LOG_TRACE("GPUStuffs: %i %i %i %i", (*GPUStuffsFetched)[0], (*GPUStuffsFetched)[1], (*GPUStuffsFetched)[2], (*GPUStuffsFetched)[3]);

    for (Entry<char*, void*> i : *arenaIndex) {
      LOG_TRACE("Key: %s", i.key);
    }

    delete arena;
  }
}
