#include <dlfcn.h>
#include "game_state.h"
#include "utils.h"

typedef void (*server_main_fn)(GameState*);

struct Server {
    void* library = nullptr;
    server_main_fn main = nullptr;
};

Server load_server() {
    LOG_TRACE("Loading server code...");
    Server server = {};
    const char* LIB_PATH = "./libserver.so";

    server.library = dlopen(LIB_PATH, RTLD_NOW | RTLD_DEEPBIND | RTLD_LOCAL) ;
    if (!server.library) {
        LOG_ERROR("Failed to open library: ", dlerror());
        return server;
    }
    LOG_TRACE("Successfully opened library");

    server.main = (server_main_fn)dlsym(server.library, "server_main");
    LOG_TRACE("Loaded server main function: ", (server.main ? "success" : "failed"));
    return server;
}

void unload_server(Server* server) {
    LOG_TRACE("Unloading library handle: ", server->library)
    dlclose(server->library);
    server->library = nullptr;
    server->main = nullptr;
}

int main() {
    LOG_TRACE("Starting server...");
    
    Server server = load_server();
    
    if (!server.library || !server.main) {
        LOG_ERROR("Failed to load initial server code");
        return 1;
    }

    GameState state = {};

    while(1) {
        server.main(&state);
        unload_server(&server);
        server = load_server();
        LOG_TRACE("Reload complete");
    }

    unload_server(&server);
}
