#ifndef KNIGHT_H
#define KNIGHT_H

#include <string>
#include <tuple>
#include <unordered_map>
#include "../json.hpp"

std::tuple<int, int, float, std::string> knight_main(const nlohmann::json& unit_state, std::unordered_map<std::string, std::string>& memory);

#endif
