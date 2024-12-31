#include "game.hpp"
#include "entt.hpp"
#include "game_state.hpp"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "raygui.h"

#define GUI_GUI_IMPLEMENTATION
#include "gui.h"

#include <iostream>

void game_init_window() {
    InitWindow(800, 450, "video game");
    SetTargetFPS(60);
}

void game_render(GameState* state) {
    try {
        BeginDrawing();
            // ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
            ClearBackground(YELLOW);
            BeginMode2D(state->persistent.camera);
                // Draw grid
                rlPushMatrix();
                    rlTranslatef(0, 25*50, 0);
                    rlRotatef(90, 1, 0, 0);
                    DrawGrid(100, 50);
                rlPopMatrix();

                // Draw all entities with Position and Renderable components
                auto view = state->persistent.registry.view<const Position, const Renderable>();
                view.each([](const Position& pos, const Renderable& rend) {
                    DrawCircleV(pos.pos, rend.radius, rend.color);
                });
            EndMode2D();

            // Draw mouse reference
            Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), state->persistent.camera);
            DrawCircleV(GetMousePosition(), 4, WHITE);
            DrawTextEx(GetFontDefault(), 
                TextFormat("[%.0f, %.0f]", mousePos.x, mousePos.y),
                Vector2Add(GetMousePosition(), (Vector2){ -44, -24 }), 20, 2, WHITE);
            GuiGui(&state->persistent.gui_state);
        EndDrawing();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in game_render: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown exception in game_render" << std::endl;
    }
}

void game_close_window() {
    CloseWindow();
}

void game_init(GameState* state) {
    std::cout << "Initializing game state..." << std::endl;
    
    // Only run on first start (not on hot reload)
    if(state->persistent.registry.storage<entt::entity>().empty()) {
        // Initialize camera
        state->persistent.camera = Camera2D{0};
        state->persistent.camera.zoom = 1.0f;
        state->persistent.camera.offset = (Vector2){400, 225};
        
        // Create some entities
        for(int i = 0; i < 10; i++) {
            auto entity = state->persistent.registry.create();
            state->persistent.registry.emplace<Position>(entity, Vector2{
                static_cast<float>(GetRandomValue(0, 800)),
                static_cast<float>(GetRandomValue(0, 450))
            });
            state->persistent.registry.emplace<Renderable>(entity, 
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
        state->persistent.gui_state = gui_state;
    }

    std::cout << "Game initialization complete" << std::endl;
}

bool game_should_close(void) {
    return WindowShouldClose();
}

void game_update(GameState* state) {
    if (!state) {
        std::cerr << "Error: Null state in game_update" << std::endl;
        return;
    }

    auto& camera = state->persistent.camera;

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
