#pragma once
#include "entt.hpp"
#include "raylib.h"
#include "raygui.h"
#include "gui.h"

struct PersistentState {
    Camera2D camera;
    entt::registry registry;
    GuiGuiState gui_state;
};

struct GameState {
    PersistentState persistent;
};

// Components
struct Position {
    Vector2 pos;
};

struct Renderable {
    Color color;
    float radius;
};
