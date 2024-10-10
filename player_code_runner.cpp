#include <iostream>
#include <string>
#include <unordered_map>

#include "player2/knight.h"

int main(int argc, char** argv) {
    std::string unit_state;
    std::string unit_id;
    std::string player_number = argv[1];
    
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> memory;
    memory["unit1"] = std::unordered_map<std::string, std::string>();
    memory["unit2"] = std::unordered_map<std::string, std::string>();
    memory["unit3"] = std::unordered_map<std::string, std::string>();

    while (std::getline(std::cin, unit_state)) {
        unit_id = unit_state;

        // TODO: Recognize what unit I am from unit_state, call the correct function

        std::tuple<int, int, float, std::string> instruction = knight_main(unit_state, memory["unit1"]);
        std::cout << unit_id << ":" << "attack" << std::endl;
        std::cout.flush();
    }

    return 0;
}
