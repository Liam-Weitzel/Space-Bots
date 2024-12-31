#include <dlfcn.h>
#include <sys/stat.h>
#include <iostream>
#include "game_state.hpp"

typedef void (*game_init_fn)(GameState*);
typedef void (*game_update_fn)(GameState*);
typedef void (*game_render_fn)(GameState*);
typedef void (*game_init_window_fn)(void);
typedef bool (*game_should_close_fn)(void);
typedef void (*game_close_window_fn)(void);

struct GameCode {
    void* library = nullptr;
    game_init_fn init = nullptr;
    game_update_fn update = nullptr;
    game_render_fn render = nullptr;
    game_init_window_fn init_window = nullptr;
    game_should_close_fn should_close = nullptr;
    game_close_window_fn close_window = nullptr;
    time_t last_write_time = 0;
};

time_t get_last_write_time(const char* filename) {
    struct stat file_stat;
    if (stat(filename, &file_stat) == 0) {
        return file_stat.st_mtime;
    }
    return 0;
}

GameCode load_game_code() {
    GameCode code = {};
    const char* LIB_PATH = "./libgame.so";

    std::cout << "[DEBUG] Starting load_game_code()" << std::endl;
    
    code.library = dlopen(LIB_PATH, RTLD_NOW | RTLD_DEEPBIND | RTLD_LOCAL) ;
    if (!code.library) {
        std::cerr << "[ERROR] Failed to open library: " << dlerror() << std::endl;
        return code;
    }
    std::cout << "[DEBUG] Successfully opened library" << std::endl;

    // Load all function pointers
    std::cout << "[DEBUG] Loading function pointers..." << std::endl;
    code.init_window = (game_init_window_fn)dlsym(code.library, "game_init_window");
    std::cout << "[DEBUG] Loaded init_window: " << (code.init_window ? "success" : "failed") << std::endl;
    
    code.init = (game_init_fn)dlsym(code.library, "game_init");
    std::cout << "[DEBUG] Loaded init: " << (code.init ? "success" : "failed") << std::endl;
    
    code.update = (game_update_fn)dlsym(code.library, "game_update");
    std::cout << "[DEBUG] Loaded update: " << (code.update ? "success" : "failed") << std::endl;
    
    code.render = (game_render_fn)dlsym(code.library, "game_render");
    std::cout << "[DEBUG] Loaded render: " << (code.render ? "success" : "failed") << std::endl;
    
    code.should_close = (game_should_close_fn)dlsym(code.library, "game_should_close");
    std::cout << "[DEBUG] Loaded should_close: " << (code.should_close ? "success" : "failed") << std::endl;
    
    code.close_window = (game_close_window_fn)dlsym(code.library, "game_close_window");
    std::cout << "[DEBUG] Loaded close_window: " << (code.close_window ? "success" : "failed") << std::endl;
    
    if (const char* error = dlerror()) {
        std::cerr << "[ERROR] Failed to load functions: " << error << std::endl;
        dlclose(code.library);
        code = {};
    } else {
        std::cout << "[DEBUG] All functions loaded successfully" << std::endl;
    }
    
    code.last_write_time = get_last_write_time(LIB_PATH);
    std::cout << "[DEBUG] Library last write time: " << code.last_write_time << std::endl;
    
    return code;
}


void unload_game_code(GameCode* code) {
    std::cout << "[DEBUG] Unloading library handle: " << code->library << std::endl;
    
    dlclose(code->library);
    
    // Clear all function pointers
    code->library = nullptr;
    code->init = nullptr;
    code->update = nullptr;
    code->render = nullptr;
    code->init_window = nullptr;
    code->should_close = nullptr;
    code->close_window = nullptr;
    code->last_write_time = 0;
}

int main() {
    std::cout << "Starting game..." << std::endl;
    
    GameCode game = load_game_code();
    
    if (!game.library || !game.init || !game.update || !game.render || 
        !game.init_window || !game.should_close) {
        std::cerr << "Failed to load initial game code" << std::endl;
        return 1;
    }

    game.init_window();
    GameState state = {};
    game.init(&state);

    while (!game.should_close()) {
        if (game.update) game.update(&state);
        if (game.render) game.render(&state);

        time_t last_write_time = get_last_write_time("./libgame.so");
        if (last_write_time != game.last_write_time) {
            game.close_window();
            unload_game_code(&game);
            game = load_game_code();
            game.init_window();
            game.init(&state);
            std::cout << "Reload complete" << std::endl;
        }
    }

    unload_game_code(&game);
    game.close_window();
    return 0;
}
