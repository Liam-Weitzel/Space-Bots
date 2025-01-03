#include <dlfcn.h>
#include "game_state.h"
#include "utils.h"

typedef void (*game_main_fn)(GameState*);

struct Game {
    void* library = nullptr;
    game_main_fn main = nullptr;
};

Game load_game() {
    LOG_TRACE("Loading game code...");
    Game game = {};
    const char* LIB_PATH = "./libgame.so";

    game.library = dlopen(LIB_PATH, RTLD_NOW | RTLD_DEEPBIND | RTLD_LOCAL) ;
    if (!game.library) {
        LOG_ERROR("Failed to open library: ", dlerror());
        return game;
    }
    LOG_TRACE("Successfully opened library");

    game.main = (game_main_fn)dlsym(game.library, "game_main");
    LOG_TRACE("Loaded game main function: ", (game.main ? "success" : "failed"));
    return game;
}

void unload_game(Game* game) {
    LOG_TRACE("Unloading library handle: ", game->library)
    dlclose(game->library);
    game->library = nullptr;
    game->main = nullptr;
}

int main() {
    LOG_TRACE("Starting game...");
    
    Game game = load_game();
    
    if (!game.library || !game.main) {
        LOG_ERROR("Failed to load initial game code");
        return 1;
    }

    GameState state = {};

    while(1) {
        game.main(&state);
        unload_game(&game);
        game = load_game();
        LOG_TRACE("Reload complete");
    }

    unload_game(&game);
}
