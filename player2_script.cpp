#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    std::string game_state;
    std::getline(std::cin, game_state);

    std::string action = "MOVE_RIGHT";

    std::cout << action << std::endl;

    return 0;
}
