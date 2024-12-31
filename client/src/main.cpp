#include <dlfcn.h>
#include <sys/stat.h>
#include <iostream>
#include "game_state.hpp"

typedef void (*game_main_fn)(GameState*);

struct Game {
    void* library = nullptr;
    game_main_fn main = nullptr;
};

Game load_game() {
    std::cout << "Loading game code..." << std::endl;
    Game game = {};
    const char* LIB_PATH = "./libgame.so";

    game.library = dlopen(LIB_PATH, RTLD_NOW | RTLD_DEEPBIND | RTLD_LOCAL) ;
    if (!game.library) {
        std::cerr << "[ERROR] Failed to open library: " << dlerror() << std::endl;
        return game;
    }
    std::cout << "Successfully opened library" << std::endl;

    game.main = (game_main_fn)dlsym(game.library, "game_main");
    std::cout << "Loaded game main function: " << (game.main ? "success" : "failed") << std::endl;
    return game;
}

void unload_game(Game* game) {
    std::cout << "Unloading library handle: " << game->library << std::endl;
    dlclose(game->library);
    game->library = nullptr;
    game->main = nullptr;
}

int main() {
    std::cout << "Starting game..." << std::endl;
    
    Game game = load_game();
    
    if (!game.library || !game.main) {
        std::cerr << "Failed to load initial game code" << std::endl;
        return 1;
    }

    GameState state = {};

    while(1) {
        game.main(&state);
        unload_game(&game);
        game = load_game();
        std::cout << "Reload complete" << std::endl;
    }

    unload_game(&game);
}
