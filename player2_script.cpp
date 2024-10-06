#include <iostream>
#include <string>

int main() {
    std::string game_state;
    
    while (std::getline(std::cin, game_state)) {
        std::string action;
        std::stoi(game_state)%2 == 0 ? action = "MOVE_RIGHT" : action = "MOVE_LEFT";
        std::cout << action << std::endl;
    }

    return 0;
}
