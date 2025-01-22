#pragma once
#include "entt.hpp"
#include "raylib.h"
#include "raygui.h"
#include "gui.h"
#include "rlights.h"

struct GUI {
    GuiGuiState gui_state;
    int style;
};

struct GameState {
    Camera camera;
    entt::registry registry;
    GUI gui;
};
