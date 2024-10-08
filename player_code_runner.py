import sys
from enum import Enum

class Action(Enum):
    pass

class KnightAction(Action):
    attack = 'attack'
    block = 'block'
    move = 'move'
    idle = 'idle'

class Direction():
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __str__(self):
        return str(self.x) + "," + str(self.y) #TODO: Some fast library to make this cleaner?

class Instruction():
    def __init__(self, action: Action):
        self.action = action
        self.direction = None

    def set_direction(self, direction: Direction):
        self.direction = direction

    def get_direction(self) -> Direction:
        return self.direction

    def __str__(self):
        return "{\"action\":\""+self.action.value+"\",\"direction\":\""+str(self.direction)+"\"}"

class KnightInstruction(Instruction):
    def __init__(self, action: Action):
        super().__init__(action)
        self.is_super_attack = False

    def set_super_attack(self, flag: bool):
        self.is_super_attack = flag

    def get_super_attack(self) -> bool:
        return self.is_super_attack

    def __str__(self):
        return super().__str__()[:-1] + ",\"is_super_attack\":\""+str(self.is_super_attack)+"\"}"

class KnightGameStateParser():
    def __init__(self):
        self.game_state = None
        self.player_number = None

    def set_game_state(self, game_state: str):
        self.game_state = game_state

    def get_game_state(self) -> str:
        return self.game_state

    def get_instruction(self):
        raise NotImplementedError("Please Implement this method")

class Knight(KnightGameStateParser):
    def get_instruction(self) -> Instruction:
        action = KnightAction.idle
        if int(self.game_state)%2==0:
            action = KnightAction.attack
        else:
            action = KnightAction.block

        instruction = KnightInstruction(action)
        instruction.set_direction(Direction(0, 1))
        instruction.set_super_attack(True)

        return instruction

#TODO: Do I want to filter in the orchistrator/ game engine side or here?
#      Passing one big JSON is quicker than passing lots of small ones...
#      Passing lots of small ones is cleaner because of seperation of concerns
def filter_game_state(game_state: str, i: str) -> str:
    return game_state

def main():
    tick = 0;
    knight = Knight() #TODO: INSTEAD OF A SINGLETON WE NEED TO MAKE SURE EACH UNIT IS ITS OWN INSTANCE THAT GETS REUSED

    while True:
        game_state = sys.stdin.readline().strip()
        player_number = sys.argv[1];
        tick += 1

        #TODO: For every programmable object in the game_state,
        #      call the relevant function implemented by the player,
        #      look in the player_number directory...
        #      and only pass it that objects info. + some deets like
        #      where home is etc. Returns an action for that unit to do

        knight.set_game_state(filter_game_state(game_state, player_number))
        print( "instruction" + str(tick) + ":" + str(knight.get_instruction())) #NOTE: tick is just printed in order for orchistrator to pick up the right print statement
        sys.stdout.flush()

if __name__=="__main__":
    main()
