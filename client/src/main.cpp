#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define GUI_GUI_IMPLEMENTATION
#include "gui.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    
    GuiLoadStyle("src/style.rgs");
    GuiLoadIcons("src/icons.rgi", "icons");
    GuiGuiState state = InitGuiGui();

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Update
        // Translate based on mouse right click
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f/camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        // Zoom based on mouse wheel
        float wheel = GetMouseWheelMove();
        if (wheel != 0)
        {
            // Get the world point that is under the mouse
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

            // Set the offset to where the mouse is
            camera.offset = GetMousePosition();

            // Set the target to match, so that the camera maps the world space point 
            // under the cursor to the screen space point under the cursor at any zoom
            camera.target = mouseWorldPos;

            // Zoom increment
            float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
            if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
            camera.zoom = Clamp(camera.zoom*scaleFactor, 0.125f, 64.0f);
        }

        //Draw
        BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
            BeginMode2D(camera);
                // Draw the 3d grid, rotated 90 degrees and centered around 0,0 
                // just so we have something in the XY plane
                rlPushMatrix();
                    rlTranslatef(0, 25*50, 0);
                    rlRotatef(90, 1, 0, 0);
                    DrawGrid(100, 50);
                rlPopMatrix();

                // Draw a reference circle
                DrawCircle(GetScreenWidth()/2, GetScreenHeight()/2, 50, YELLOW);
            EndMode2D();
            
            // Draw mouse reference
            Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
            DrawCircleV(GetMousePosition(), 4, DARKGRAY);
            DrawTextEx(GetFontDefault(), 
                TextFormat("[%.0f, %.0f]", mousePos.x, mousePos.y),
                Vector2Add(GetMousePosition(), (Vector2){ -44, -24 }), 20, 2, BLACK);

            GuiGui(&state);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
