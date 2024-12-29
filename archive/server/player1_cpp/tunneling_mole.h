#ifndef TUNNELING_MOLE_H
#define TUNNELING_MOLE_H

#include <string>
#include <tuple>
#include <unordered_map>
#include "../json.hpp"

std::tuple<int, int, float, std::string> tunneling_mole(const nlohmann::json& unit_state, std::unordered_map<std::string, std::string>& memory);

#endif
