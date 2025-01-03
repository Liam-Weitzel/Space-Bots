#include "client.h"
#include "game_state.h"

#include "entt.hpp"

#include "utils.h"

#include "isteamuser.h"
#include "isteamuserstats.h"
#include "steam_api.h"
#include "steamtypes.h"

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "raygui.h"

#define GUI_GUI_IMPLEMENTATION
#include "gui.h"

void render(GameState* state) {
    BeginDrawing();
        BeginMode2D(state->camera);
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            // Draw grid
            rlPushMatrix();
                rlTranslatef(0, 25*50, 0);
                rlRotatef(90, 1, 0, 0);
                DrawGrid(100, 50);
            rlPopMatrix();

            // Draw all entities with Position and Renderable components
            auto view = state->registry.view<const Position, const Renderable>();
            view.each([](const Position& pos, const Renderable& rend) {
                DrawCircleV(pos.pos, rend.radius, rend.color);
            });
        EndMode2D();

        // Draw mouse reference
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), state->camera);
        DrawCircleV(GetMousePosition(), 4, GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)));
        DrawTextEx(GuiGetFont(), 
            TextFormat("[%.0f, %.0f]", mousePos.x, mousePos.y),
            Vector2Add(GetMousePosition(), (Vector2){ -44, -24 }), 20, 2, GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)));

        GuiGui(&state->gui);
    EndDrawing();
}

void init(GameState* state) {
    // Initialize Steam API
    if (SteamAPI_Init()) {
        LOG_TRACE("Steam API initialized successfully!");

        // Check if the user is logged into Steam
        if (SteamUser()->BLoggedOn()) {
            const char* playerName = SteamFriends()->GetPersonaName();
            const uint64 steamID = SteamUser()->GetSteamID().ConvertToUint64();
            
            // Prepare buffer for auth ticket
            const int MAX_TICKET_SIZE = 1024;
            unsigned char ticketBuffer[MAX_TICKET_SIZE];
            uint32 ticketSize;
            
            // Set up remote server identity
            // SteamNetworkingIdentity serverIdentity;
            // serverIdentity.Clear();
            // serverIdentity.SetSteamID64(YOUR_SERVER_STEAM_ID);

            // Get auth session ticket with server identity
            HAuthTicket sessionTicket = SteamUser()->GetAuthSessionTicket(
                ticketBuffer, 
                MAX_TICKET_SIZE, 
                &ticketSize,
                // &serverIdentity  // Include server identity
                nullptr
            );

            if (sessionTicket != k_HAuthTicketInvalid) {
                LOG_TRACE("Steam user logged in: %u", steamID);
                // std::cout << "Auth ticket obtained for server " << serverIdentity.GetSteamID64() << std::endl;
                LOG_TRACE("Ticket size: %u", ticketSize);
            } else {
                LOG_ERROR("Failed to get auth ticket")
            }
        } else {
            LOG_ASSERT(false, "Steam user not logged in")
        }
    } else {
        LOG_ASSERT(false, "Steam API initialization failed!");
    }

    const uint32 num_achievements = SteamUserStats()->GetNumAchievements();
    LOG_TRACE("num achievements: %u", num_achievements);

    //if we are launching for the first time: NOT HOT RELOADED
    if(state->registry.storage<entt::entity>().empty()) {
        state->camera = Camera2D{0};
        state->camera.zoom = 1.0f;
        state->camera.offset = (Vector2){400, 225};

        // Create some entities
        for(int i = 0; i < 10; i++) {
            auto entity = state->registry.create();
            state->registry.emplace<Position>(entity, Vector2{
                static_cast<float>(GetRandomValue(0, 800)),
                static_cast<float>(GetRandomValue(0, 450))
            });
            state->registry.emplace<Renderable>(entity, 
                Color{
                    static_cast<unsigned char>(GetRandomValue(0, 255)),
                    static_cast<unsigned char>(GetRandomValue(0, 255)),
                    static_cast<unsigned char>(GetRandomValue(0, 255)),
                    255
                },
                static_cast<float>(GetRandomValue(10, 30))
            );
        }

        GuiLoadStyleDefault();
        GuiLoadIcons("assets/icons.rgi", "icons");
        GuiGuiState gui_state = InitGuiGui();
        state->gui.gui_state = gui_state;
    }
}

void update(GameState* state) {
    auto& camera = state->camera;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/camera.zoom);
        camera.target = Vector2Add(camera.target, delta);
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector2 mousePos = GetMousePosition();
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        camera.offset = GetMousePosition();
        camera.target = mouseWorldPos;
        float scaleFactor = 1.0f + (0.25f*fabsf(wheel));

        if (wheel < 0) {
            scaleFactor = 1.0f/scaleFactor;
        }
        float newZoom = camera.zoom*scaleFactor;
        camera.zoom = Clamp(newZoom, 0.5f, 5.0f);
    }
}

extern "C" void client_main(GameState* state) {
    InitWindow(800, 450, "video game");
    SetTargetFPS(60);
    init(state);
    time_t last_write_time = get_timestamp("./libclient.so"); //comment out for prod build
    while(!WindowShouldClose()) {
        if(last_write_time != get_timestamp("./libclient.so")) break; //comment for prod build
        update(state);
        render(state);
    }
    SteamAPI_Shutdown();
    CloseWindow();
}
