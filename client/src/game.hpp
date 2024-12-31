#pragma once
#include "game_state.hpp"

extern "C" {
    void game_init(GameState* state);
    void game_update(GameState* state);
    void game_render(GameState* state);
    void game_init_window(void);
    bool game_should_close(void);
    void game_close_window(void);
}
