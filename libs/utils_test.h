#pragma once

#include "utils.h"

// Example Entity
struct Entity {
  int id;
  char* name;
};

void arena_create_and_fetch_in_different_scope_test();
void arena_e2e_test();
