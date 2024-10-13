import subprocess
import sys
import init_game_state
import json

def start_player_script(lang, player_number):
    if lang == "cpp":
        output_binary = "player" + str(player_number) + "_out"
        subprocess.run(["g++", "player_code_runner.cpp", "player1/knight.cpp", "json.hpp", "-o", output_binary], capture_output=True)
        return subprocess.Popen(["./"+output_binary, str(player_number)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    elif lang == "py":
        return subprocess.Popen([sys.executable, "player_code_runner.py", str(player_number)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

def split_game_state_into_unit_states(game_state):
    units = game_state.get("units", [])
    unit_states = []

    for current_unit in units:
        unit_dict = {
            'self': current_unit,
            'units': []
        }

        # Add all other units (excluding the current one) to the 'units' list
        unit_dict['units'] = [unit for unit in units if unit['id'] != current_unit['id']]
        unit_states.append(unit_dict)
    return unit_states

def send_unit_state(player_process, unit_state, unit_id): #TODO: Add timeout
    player_process.stdin.write(unit_state + "\n")
    player_process.stdin.flush()
    output_lines = []
    action = None

    while True:
        line = player_process.stdout.readline().strip()

        if line:
            if line.startswith(f"{unit_id}:"):
                action = line
                break
            else:
                output_lines.append(line)

    return action, output_lines

player0_process = start_player_script("py", 0)
player1_process = start_player_script("cpp", 1)

ticklog = []
game_state = init_game_state.main()

for tick in range(10000):
    #game_state = game_engine(game_state, ticklog)
    unit_states = split_game_state_into_unit_states(game_state)
    actions = []

    for unit_state in unit_states:
        id = unit_state.get("self").get("id")
        player = unit_state.get("self").get("player")
        unit_state = json.dumps(unit_state)
        if(player == 0):
            action = send_unit_state(player0_process, unit_state, id)
        else: 
            action = send_unit_state(player1_process, unit_state, id)
        actions.append(action)

    ticklog.append([tick, actions])

print(ticklog)

player0_process.terminate()
player1_process.terminate()
