#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

#include "json.hpp"

#include "player1/acid_ant.h"
#include "player1/bloated_bedbug.h"
#include "player1/dung_beetle.h"
#include "player1/engorged_tick.h"
#include "player1/famished_tick.h"
#include "player1/foraging_maggot.h"
#include "player1/infected_mouse.h"
#include "player1/lava_ant.h"
#include "player1/mantis.h"
#include "player1/mawing_beaver.h"
#include "player1/plague_bat.h"
#include "player1/rhino_beetle.h"
#include "player1/swooping_bat.h"
#include "player1/tainted_cockroach.h"
#include "player1/tunneling_mole.h"

using UnitFunction = std::function<std::tuple<int, int, float, std::string>(const nlohmann::json&, std::unordered_map<std::string, std::string>&)>;

int main(int argc, char** argv) {
    std::string unit_state_str;
    int unit_id;
    std::string player_number = argv[1];
    nlohmann::json unit_state;

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
        {"swooping_bat", swooping_bat},
        {"tainted_cockroach", tainted_cockroach},
        {"tunneling_mole", tunneling_mole}
    };

    while (std::getline(std::cin, unit_state_str)) {
        unit_state = nlohmann::json::parse(unit_state_str);
        unit_id = unit_state["self"]["id"];
        std::string unit_type = unit_state["self"]["type"];

        auto it = unit_functions.find(unit_type);
        if (it != unit_functions.end()) {

            UnitFunction unit_function = it->second;
            std::tuple<int, int, float, std::string> instruction = unit_function(unit_state, memory[unit_id]);

            std::cout << unit_id << ":" << "attack" << std::endl;
            std::cout.flush();
        } else {
            std::cerr << "Unknown unit type: " << unit_type << std::endl;
        }
    }

    return 0;
}
