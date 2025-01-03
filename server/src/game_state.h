#pragma once
#include "entt.hpp"
#include "utils.h"

struct GameState {
    Camera2D camera;
    entt::registry registry;
};

// Components
struct Position {
    Vector2 pos;
};

struct Renderable {
    Color color;
    float radius;
};
