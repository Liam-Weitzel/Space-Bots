#include <dlfcn.h>
#include "game_state.h"
#include "utils_test.h"
#include "utils.h"

// typedef void (*client_main_fn)(GameState*);

struct Client {
    void* library = nullptr;
    // client_main_fn main = nullptr;
};

Client load_client() {
    LOG_TRACE("Loading client code...");
    Client client = {};
    const char* LIB_PATH = "./libclient.so";

    client.library = dlopen(LIB_PATH, RTLD_NOW | RTLD_DEEPBIND | RTLD_LOCAL);
    if (!client.library) {
        LOG_ERROR("Failed to open library: ", dlerror());
        return client;
    }
    LOG_TRACE("Successfully opened library");

    // client.main = (client_main_fn)dlsym(client.library, "client_main");
    // LOG_TRACE("Loaded client main function: ", (client.main ? "success" : "failed"));
    return client;
}

void unload_client(Client* client) {
    LOG_TRACE("Unloading library handle: ", client->library)
    dlclose(client->library);
    client->library = nullptr;
    // client->main = nullptr;
}

int main() {
    LOG_TRACE("Starting client...");
    
    Client client = load_client();
    
    if (!client.library) {
        LOG_ERROR("Failed to load initial client code");
        return 1;
    }

    // GameState state = {};
    // client.main(&state);

    iterators_arrays_CT_test();
    iterators_arrays_RT_test();
    create_and_fetch_arena_in_different_scope_CT_test();
    create_and_fetch_arena_in_different_scope_RT_test();
    create_hashmap_in_arena_CT_test();
    create_hashmap_in_arena_RT_test();
    quicksort_test();
    ecs_test();
    create_arena_clear_test();
    file_io_test();

    unload_client(&client);
}
