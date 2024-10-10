import sys
from enum import Enum
from player1 import knight

def main():
    memory = [{}, {}, {}] #NOTE: one dict per game object, pass the right one into the object

    while True:
        unit_state = sys.stdin.readline().strip()
        unit_id = unit_state
        player_number = sys.argv[1]

        #TODO: recognize what unit I am fron unit_state, call the correct function

        instruction = knight.main(unit_state, memory[0])
        print(f"{unit_id}:{instruction}")
        sys.stdout.flush()

if __name__=="__main__":
    main()
