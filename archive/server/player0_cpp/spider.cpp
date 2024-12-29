#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include "../json.hpp"

std::tuple<int, int, float, std::string> spider(const nlohmann::json& unit_state, std::unordered_map<std::string, std::string>& memory) {
    int x_direction = 1; // Direction_x and direction_y dictate which direction your action will be executed in.
    int y_direction = 0; // The sum of direction x and y cannot be greater than 1 and not lower than -1.
    float velocity = 0.5f; // Only required for move and skill shots
    std::string action = "move"; // Possible actions: move, attack, block

    // Your code goes here
    std::cout << "running from cpp" << std::endl;

    return std::make_tuple(x_direction, y_direction, velocity, action);
}
