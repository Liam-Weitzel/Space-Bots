#ifndef FAMISHED_TICK_H
#define FAMISHED_TICK_H

#include <string>
#include <tuple>
#include <unordered_map>
#include "../json.hpp"

std::tuple<int, int, float, std::string> famished_tick(const nlohmann::json& unit_state, std::unordered_map<std::string, std::string>& memory);

#endif
