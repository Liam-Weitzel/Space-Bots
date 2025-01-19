#ifndef LAVA_ANT_H
#define LAVA_ANT_H

#include <string>
#include <tuple>
#include <unordered_map>
#include "../json.hpp"

std::tuple<int, int, float, std::string> lava_ant(const nlohmann::json& unit_state, std::unordered_map<std::string, std::string>& memory);

#endif
