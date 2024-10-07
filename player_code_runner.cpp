#include <iostream>
#include <string>

int main(int argc, char** argv) {
    std::string game_state;
    std::string player_number = argv[1];
    
    while (std::getline(std::cin, game_state)) {
        //TODO: For every programmable object in the game_state,
        //      call the relevant function implemented by the player,
        //      look in the player_number directory...
        //      and only pass it that objects info. + some deets like
        //      where home is etc. Returns an action for that unit to do
        std::string action;
        std::stoi(game_state)%2 == 0 ? action = "MOVE_RIGHT" : action = "MOVE_LEFT";
        std::cout << action << player_number << std::endl;
    }

    return 0;
}
