#pragma once

#include "utils.h"

// Example Entity
struct Entity {
  int id;
  char* name;
  
  bool operator==(Entity other) {
    if(this->id == other.id && this->name == other.name) return true;
    return false;
  }

  bool operator!=(Entity other) {
    if(this->id == other.id && this->name == other.name) return false;
    return true;
  }

  bool operator<=(const Entity& other) const {
      return id <= other.id;
  }
  
  bool operator>=(const Entity& other) const {
      return id >= other.id;
  }
  
  bool operator<(const Entity& other) const {
      return id < other.id;
  }
  
  bool operator>(const Entity& other) const {
      return id > other.id;
  }
};

// NOTE: Datastructures
void iterators_arrays_CT_test();
void iterators_arrays_RT_test();
void create_and_fetch_arena_in_different_scope_CT_test();
void create_and_fetch_arena_in_different_scope_RT_test();
void create_hashmap_in_arena_CT_test();
void create_hashmap_in_arena_RT_test();
void quicksort_test();
void ecs_test();
void create_arena_clear_test();

// NOTE: File I/O
void file_io_test();
