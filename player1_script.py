import sys

while True:
    game_state = sys.stdin.readline().strip()
    action = "??"

    if int(game_state)%2==0:
        action = "ATTACK"
    else:
        action = "DEFEND"

    print(action)
    sys.stdout.flush()
