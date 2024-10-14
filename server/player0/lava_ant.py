def lava_ant(unit_state: dict, memory: dict) -> (int, int, float, str):
    x_direction = 1 #Direction_x and direction_y dictate which direction your action will be executed in. e.g. move in direction x -1 & y 0 = move to the left
    y_direction = 0 #The sum of direction x and y can not be greater than 1 and not lower than -1. e.g. move diagonally to the bottom right is x 0.5 & y 0.5
    velocity = 0.5 #Only required for move and skill shots
    action = "move" #Possible actions: move, attack, block

    #Your code goes here
    print("running from python")

    return x_direction, y_direction, velocity, action
