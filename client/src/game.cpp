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
    // std::cout << "Starting game_update..." << std::endl;
    
    if (!state) {
        std::cerr << "Error: Null state in game_update" << std::endl;
        return;
    }

    auto& camera = state->persistent.camera;
    // std::cout << "Current camera state - zoom: " << camera.zoom 
    //           << ", target: (" << camera.target.x << ", " << camera.target.y << ")" 
    //           << ", offset: (" << camera.offset.x << ", " << camera.offset.y << ")" 
    //           << std::endl;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        // std::cout << "Left mouse button is down" << std::endl;
        Vector2 delta = GetMouseDelta();
        // std::cout << "Mouse delta: (" << delta.x << ", " << delta.y << ")" << std::endl;
        
        delta = Vector2Scale(delta, -1.0f/camera.zoom);
        // std::cout << "Scaled delta: (" << delta.x << ", " << delta.y << ")" << std::endl;
        
        camera.target = Vector2Add(camera.target, delta);
        // std::cout << "New camera target: (" << camera.target.x << ", " << camera.target.y << ")" << std::endl;
    }

    float wheel = GetMouseWheelMove();
    // std::cout << "Mouse wheel movement: " << wheel << std::endl;
    
    if (wheel != 0) {
        // std::cout << "Processing mouse wheel movement..." << std::endl;
        
        Vector2 mousePos = GetMousePosition();
        // std::cout << "Mouse position: (" << mousePos.x << ", " << mousePos.y << ")" << std::endl;
        
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        // std::cout << "Mouse world position: (" << mouseWorldPos.x << ", " << mouseWorldPos.y << ")" << std::endl;
        
        camera.offset = GetMousePosition();
        // std::cout << "New camera offset: (" << camera.offset.x << ", " << camera.offset.y << ")" << std::endl;
        
        camera.target = mouseWorldPos;
        // std::cout << "New camera target: (" << camera.target.x << ", " << camera.target.y << ")" << std::endl;

        float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
        // std::cout << "Initial scale factor: " << scaleFactor << std::endl;
        
        if (wheel < 0) {
            scaleFactor = 1.0f/scaleFactor;
            // std::cout << "Inverted scale factor: " << scaleFactor << std::endl;
        }
        
        float newZoom = camera.zoom*scaleFactor;
        // std::cout << "Pre-clamp zoom: " << newZoom << std::endl;
        
        camera.zoom = Clamp(newZoom, 0.5f, 5.0f);
        // std::cout << "Final camera zoom: " << camera.zoom << std::endl;
    }

    // std::cout << "game_update complete" << std::endl;
}
