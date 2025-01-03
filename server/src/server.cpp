#include "server.h"
#include "game_state.h"

#include "entt.hpp"

#include "utils.h"

#include "isteamuser.h"
#include "isteamuserstats.h"
#include "steam_api.h"
#include "steamtypes.h"

extern "C" void server_main(GameState* state) {
    LOG_TRACE("working...");
    time_t last_write_time = get_timestamp("./libserver.so"); //comment out for prod build
    while(last_write_time == get_timestamp("./libserver.so")) {
    }
}
