#include "entt.hpp"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define GUI_GUI_IMPLEMENTATION
#include "gui.h"

// Components
struct Position {
    Vector2 pos;
};

struct Renderable {
    Color color;
    float radius;
};

int main(void) {
    SetRandomSeed(time(NULL));
    const int screenWidth = 800;
    const int screenHeight = 450;

    entt::registry registry;

    // Create some random circles
    for(int i = 0; i < 10; i++) {
        auto entity = registry.create();
        registry.emplace<Position>(entity, Vector2{
            static_cast<float>(GetRandomValue(0, screenWidth)),
            static_cast<float>(GetRandomValue(0, screenHeight))
        });
        registry.emplace<Renderable>(entity, 
            Color{
                static_cast<unsigned char>(GetRandomValue(0, 255)),
                static_cast<unsigned char>(GetRandomValue(0, 255)),
                static_cast<unsigned char>(GetRandomValue(0, 255)),
                255
            },
            static_cast<float>(GetRandomValue(10, 30))
        );
    }

    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    InitWindow(screenWidth, screenHeight, "video game");
    
    GuiLoadIcons("src/icons.rgi", "icons");
    GuiGuiState state = InitGuiGui();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            // Translate camera when left click is held
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f/camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }


        float wheel = GetMouseWheelMove();
        // Zoom camera when mouse wheel moved
        if (wheel != 0) {
            // Set the offset to where the mouse is
            // Set the target to match, so that the camera maps the world space point 
            // under the cursor to the screen space point under the cursor at any zoom
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            camera.offset = GetMousePosition();
            camera.target = mouseWorldPos;

            float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
            if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
            camera.zoom = Clamp(camera.zoom*scaleFactor, 0.5f, 5.0f);
        }

        // Draw
        BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
            BeginMode2D(camera);
                // Draw grid
                rlPushMatrix();
                    rlTranslatef(0, 25*50, 0);
                    rlRotatef(90, 1, 0, 0);
                    DrawGrid(100, 50);
                rlPopMatrix();

                // Draw all entities with Position and Renderable components
                auto view = registry.view<const Position, const Renderable>();
                view.each([](const Position& pos, const Renderable& rend) {
                    DrawCircleV(pos.pos, rend.radius, rend.color);
                });

            EndMode2D();
            
            // Draw mouse reference
            Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
            DrawCircleV(GetMousePosition(), 4, WHITE);
            DrawTextEx(GetFontDefault(), 
                TextFormat("[%.0f, %.0f]", mousePos.x, mousePos.y),
                Vector2Add(GetMousePosition(), (Vector2){ -44, -24 }), 20, 2, WHITE);

            GuiGui(&state);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
