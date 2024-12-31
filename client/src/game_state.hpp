#pragma once
#include "entt.hpp"
#include "raylib.h"
#include "raygui.h"
#include "gui.h"

struct GUI {
    GuiGuiState gui_state;
    int style;
};

struct GameState {
    Camera2D camera;
    entt::registry registry;
    GUI gui;
    time_t last_write_time;
};

// Components
struct Position {
    Vector2 pos;
};

struct Renderable {
    Color color;
    float radius;
};
