#include "game.hpp"
#include "game_state.hpp"

#include "entt.hpp"
#include "steam_api.h"

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "raygui.h"

#define GUI_GUI_IMPLEMENTATION
#include "gui.h"

#include <sys/stat.h>
#include <iostream>

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
        DrawCircleV(GetMousePosition(), 4, WHITE);
        DrawTextEx(GetFontDefault(), 
            TextFormat("[%.0f, %.0f]", mousePos.x, mousePos.y),
            Vector2Add(GetMousePosition(), (Vector2){ -44, -24 }), 20, 2, WHITE);

        GuiGui(&state->gui);
    EndDrawing();
}

void init(GameState* state) {
    // Initialize Steam API
    if (SteamAPI_Init()) {
        std::cout << "Steam API initialized successfully!" << std::endl;

        // Check if the user is logged into Steam
        if (SteamUser()->BLoggedOn()) {
            const char* playerName = SteamFriends()->GetPersonaName();
            std::cout << "Steam user logged in: " << playerName << std::endl;
        } else {
            std::cerr << "Steam user not logged in." << std::endl;
        }
    } else {
        std::cerr << "Steam API initialization failed!" << std::endl;
    }

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

time_t get_last_write_time() {
    struct stat file_stat;
    if (stat("./libgame.so", &file_stat) == 0) {
        return file_stat.st_mtime;
    }
    return 0;
}

extern "C" void game_main(GameState* state) {
    InitWindow(800, 450, "video game");
    SetTargetFPS(60);
    init(state);
    state->last_write_time = get_last_write_time(); //comment out for prod build
    while(!WindowShouldClose()) {
        if(state->last_write_time != get_last_write_time()) break; //comment for prod build
        update(state);
        render(state);
    }
    SteamAPI_Shutdown();
    CloseWindow();
}
