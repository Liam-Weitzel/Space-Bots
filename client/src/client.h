#pragma once
#include "game_state.h"

void init(GameState* state);
void update(GameState* state);
void render(GameState* state);
extern "C" void game_main(GameState* state);
