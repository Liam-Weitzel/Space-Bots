#include <dlfcn.h>
#include "game_state.h"
#include "utils.h"

typedef void (*client_main_fn)(GameState*);

struct Client {
    void* library = nullptr;
    client_main_fn main = nullptr;
};

Client load_client() {
    LOG_TRACE("Loading client code...");
    Client client{};

    client.library = dlopen(CLIENT_LIB_PATH, RTLD_NOW);
    if (!client.library) {
        LOG_ERROR("Failed to open library: ", dlerror());
        return client;
    }
    LOG_TRACE("Successfully opened library");

    client.main = (client_main_fn)dlsym(client.library, "client_main");
    LOG_TRACE("Loaded client main function: ", (client.main ? "success" : "failed"));
    return client;
}

void unload_client(Client* client) {
    LOG_TRACE("Unloading library handle: ", client->library)
    dlclose(client->library);
    client->library = nullptr;
    client->main = nullptr;
}

int main() {
    LOG_TRACE("Starting client...");
    
    Client client = load_client();
    
    if (!client.library || !client.main) {
        LOG_ERROR("Failed to load initial client code");
        return 1;
    }

    GameState state{};

    while(1) {
        client.main(&state);
        unload_client(&client);
        client = load_client();
        LOG_TRACE("Reload complete");
    }

    unload_client(&client);
}
