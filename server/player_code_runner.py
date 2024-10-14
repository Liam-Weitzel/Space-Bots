import sys
from player0 import (
    acid_ant, bloated_bedbug, dung_beetle, engorged_tick, 
    famished_tick, foraging_maggot, infected_mouse, 
    lava_ant, mantis, mawing_beaver, plague_bat, rhino_beetle, 
    swooping_bat, tainted_cockroach, tunneling_mole
)
import json

def main():
    memory = {}

    unit_functions = {
        "acid_ant": acid_ant.acid_ant,
        "bloated_bedbug": bloated_bedbug.bloated_bedbug,
        "dung_beetle": dung_beetle.dung_beetle,
        "engorged_tick": engorged_tick.engorged_tick,
        "famished_tick": famished_tick.famished_tick,
        "foraging_maggot": foraging_maggot.foraging_maggot,
        "infected_mouse": infected_mouse.infected_mouse,
        "lava_ant": lava_ant.lava_ant,
        "mantis": mantis.mantis,
        "mawing_beaver": mawing_beaver.mawing_beaver,
        "plague_bat": plague_bat.plague_bat,
        "rhino_beetle": rhino_beetle.rhino_beetle,
        "swooping_bat": swooping_bat.swooping_bat,
        "tainted_cockroach": tainted_cockroach.tainted_cockroach,
        "tunneling_mole": tunneling_mole.tunneling_mole
    }

    while True:
        unit_state = json.loads(sys.stdin.readline().strip())
        unit_id = unit_state.get("self").get("id")
        unit_type = unit_state.get("self").get("type")
        player_number = sys.argv[1]

        if unit_id not in memory:
            memory[unit_id] = {}

        unit_function = unit_functions.get(unit_type)

        if unit_function:
            instruction = unit_function(unit_state, memory[unit_id])
            print(f"{unit_id}:{instruction}")
            sys.stdout.flush()
        else:
            print(f"Unknown unit type: {unit_type}", file=sys.stderr)

if __name__ == "__main__":
    main()
