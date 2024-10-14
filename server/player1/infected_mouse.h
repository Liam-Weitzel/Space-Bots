#ifndef INFECTED_MOUSE_H
#define INFECTED_MOUSE_H

#include <string>
#include <tuple>
#include <unordered_map>
#include "../json.hpp"

std::tuple<int, int, float, std::string> infected_mouse(const nlohmann::json& unit_state, std::unordered_map<std::string, std::string>& memory);

#endif
