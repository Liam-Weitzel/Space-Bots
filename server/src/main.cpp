#include <dlfcn.h>
#include "game_state.h"
#include "utils.h"

typedef void (*orchistrator_main_fn)(GameState*);

struct Orchistrator {
    void* library = nullptr;
    orchistrator_main_fn main = nullptr;
};

Orchistrator load_orchistrator() {
    LOG_TRACE("Loading orchistrator code...");
    Orchistrator orchistrator = {};
    const char* LIB_PATH = "./libserver.so";

    orchistrator.library = dlopen(LIB_PATH, RTLD_NOW | RTLD_DEEPBIND | RTLD_LOCAL) ;
    if (!orchistrator.library) {
        LOG_ERROR("Failed to open library: ", dlerror());
        return orchistrator;
    }
    LOG_TRACE("Successfully opened library");

    orchistrator.main = (orchistrator_main_fn)dlsym(orchistrator.library, "orchistrator_main");
    LOG_TRACE("Loaded orchistrator main function: ", (orchistrator.main ? "success" : "failed"));
    return orchistrator;
}

void unload_orchistrator(Orchistrator* orchistrator) {
    LOG_TRACE("Unloading library handle: ", orchistrator->library)
    dlclose(orchistrator->library);
    orchistrator->library = nullptr;
    orchistrator->main = nullptr;
}

int main() {
    LOG_TRACE("Starting orchistrator...");
    
    Orchistrator orchistrator = load_orchistrator();
    
    if (!orchistrator.library || !orchistrator.main) {
        LOG_ERROR("Failed to load initial orchistrator code");
        return 1;
    }

    GameState state = {};

    while(1) {
        orchistrator.main(&state);
        unload_orchistrator(&orchistrator);
        orchistrator = load_orchistrator();
        LOG_TRACE("Reload complete");
    }

    unload_orchistrator(&orchistrator);
}
