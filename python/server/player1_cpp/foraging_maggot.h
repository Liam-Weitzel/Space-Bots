#ifndef FORAGING_MAGGOT_H
#define FORAGING_MAGGOT_H

#include <string>
#include <tuple>
#include <unordered_map>
#include "../json.hpp"

std::tuple<int, int, float, std::string> foraging_maggot(const nlohmann::json& unit_state, std::unordered_map<std::string, std::string>& memory);

#endif
