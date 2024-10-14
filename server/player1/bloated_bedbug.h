#ifndef BLOATED_BEDBUG_H
#define BLOATED_BEDBUG_H

#include <string>
#include <tuple>
#include <unordered_map>
#include "../json.hpp"

std::tuple<int, int, float, std::string> bloated_bedbug(const nlohmann::json& unit_state, std::unordered_map<std::string, std::string>& memory);

#endif
