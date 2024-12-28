#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

#include "json.hpp"

// Macro to declare a unit function prototype
#define DECLARE_UNIT_FUNCTION(unit) \
    std::tuple<int, int, float, std::string> unit(const nlohmann::json&, std::unordered_map<std::string, std::string>&)

// Declare all unit functions
DECLARE_UNIT_FUNCTION(acid_ant);
DECLARE_UNIT_FUNCTION(bloated_bedbug);
DECLARE_UNIT_FUNCTION(dung_beetle);
DECLARE_UNIT_FUNCTION(engorged_tick);
DECLARE_UNIT_FUNCTION(famished_tick);
DECLARE_UNIT_FUNCTION(foraging_maggot);
DECLARE_UNIT_FUNCTION(infected_mouse);
DECLARE_UNIT_FUNCTION(lava_ant);
DECLARE_UNIT_FUNCTION(mantis);
DECLARE_UNIT_FUNCTION(mawing_beaver);
DECLARE_UNIT_FUNCTION(plague_bat);
DECLARE_UNIT_FUNCTION(rhino_beetle);
DECLARE_UNIT_FUNCTION(spider);
DECLARE_UNIT_FUNCTION(swooping_bat);
DECLARE_UNIT_FUNCTION(tainted_cockroach);
DECLARE_UNIT_FUNCTION(tunneling_mole);

using UnitFunction = std::function<std::tuple<int, int, float, std::string>(const nlohmann::json&, std::unordered_map<std::string, std::string>&)>;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <player_number>" << std::endl;
        return 1;
    }

    std::string player_number = argv[1];
    nlohmann::json unit_state;
    std::string unit_state_str;
    int unit_id;
    std::unordered_map<int, std::unordered_map<std::string, std::string>> memory;

    std::unordered_map<std::string, UnitFunction> unit_functions = {
        {"acid_ant", acid_ant},
        {"bloated_bedbug", bloated_bedbug},
        {"dung_beetle", dung_beetle},
        {"engorged_tick", engorged_tick},
        {"famished_tick", famished_tick},
        {"foraging_maggot", foraging_maggot},
        {"infected_mouse", infected_mouse},
        {"lava_ant", lava_ant},
        {"mantis", mantis},
        {"mawing_beaver", mawing_beaver},
        {"plague_bat", plague_bat},
        {"rhino_beetle", rhino_beetle},
        {"spider", spider},
        {"swooping_bat", swooping_bat},
        {"tainted_cockroach", tainted_cockroach},
        {"tunneling_mole", tunneling_mole}
    };

    // Main loop to process unit states
    while (std::getline(std::cin, unit_state_str)) {
        unit_state = nlohmann::json::parse(unit_state_str);
        unit_id = unit_state["self"]["id"];
        std::string unit_type = unit_state["self"]["type"];

        auto it = unit_functions.find(unit_type);
        if (it != unit_functions.end()) {
            UnitFunction unit_function = it->second;
            auto instruction = unit_function(unit_state, memory[unit_id]);

            std::cout << unit_id << ":" << std::get<3>(instruction) << std::endl; //FIX: not the right format anymore, json now
            std::cout.flush();
        } else {
            std::cerr << "Unknown unit type: " << unit_type << std::endl;
        }
    }

    return 0;
}
