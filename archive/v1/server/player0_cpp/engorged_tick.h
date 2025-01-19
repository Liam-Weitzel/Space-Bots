#ifndef ENGORGED_TICK_H
#define ENGORGED_TICK_H

#include <string>
#include <tuple>
#include <unordered_map>
#include "../json.hpp"

std::tuple<int, int, float, std::string> engorged_tick(const nlohmann::json& unit_state, std::unordered_map<std::string, std::string>& memory);

#endif
