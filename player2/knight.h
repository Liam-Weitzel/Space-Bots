#ifndef KNIGHT_H
#define KNIGHT_H

#include <string>
#include <tuple>
#include <unordered_map>

std::tuple<int, int, float, std::string> knight_main(const std::string& unit_state, std::unordered_map<std::string, std::string>& memory);

#endif
