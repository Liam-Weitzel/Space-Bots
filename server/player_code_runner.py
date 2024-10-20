import sys
import json
import importlib

def main():
    if len(sys.argv) < 2:
        print("Usage: python player_code_runner.py <player_number>", file=sys.stderr)
        sys.exit(1)

    player_number = sys.argv[1]
    player_module_name = f"player{player_number}_py"

    try:
        # Import the parent player module
        player_module = importlib.import_module(player_module_name)
    except ModuleNotFoundError:
        print(f"Player module {player_module_name} not found.", file=sys.stderr)
        sys.exit(1)

    memory = {}

    # List of unit names to dynamically import from the player module
    unit_names = [
        "acid_ant", "bloated_bedbug", "dung_beetle", "engorged_tick", 
        "famished_tick", "foraging_maggot", "infected_mouse", "lava_ant", 
        "mantis", "mawing_beaver", "plague_bat", "rhino_beetle", 
        "spider", "swooping_bat", "tainted_cockroach", "tunneling_mole"
    ]

    # Dynamically import each unit module from the player module
    unit_functions = {}
    for unit_name in unit_names:
        try:
            unit_module = importlib.import_module(f"{player_module_name}.{unit_name}")
            # Assuming each module has a function with the same name as the module
            unit_functions[unit_name] = getattr(unit_module, unit_name)
        except (ModuleNotFoundError, AttributeError):
            print(f"Failed to import {unit_name} from {player_module_name}.", file=sys.stderr)

    while True:
        unit_state = json.loads(sys.stdin.readline().strip())
        unit_id = unit_state.get("self").get("id")
        unit_type = unit_state.get("self").get("type")

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
