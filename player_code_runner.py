import sys

while True:
    game_state = sys.stdin.readline().strip()
    player_number = sys.argv[1];
    action = "??"

    #TODO: For every programmable object in the game_state,
    #      call the relevant function implemented by the player,
    #      look in the player_number directory...
    #      and only pass it that objects info. + some deets like
    #      where home is etc. Returns an action for that unit to do
    if int(game_state)%2==0:
        action = "ATTACK"
    else:
        action = "DEFEND"

    print(action + player_number)
    sys.stdout.flush()
