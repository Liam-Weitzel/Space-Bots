#ifndef DUNG_BEETLE_H
#define DUNG_BEETLE_H

#include <string>
#include <tuple>
#include <unordered_map>
#include "../json.hpp"

std::tuple<int, int, float, std::string> dung_beetle(const nlohmann::json& unit_state, std::unordered_map<std::string, std::string>& memory);

#endif
