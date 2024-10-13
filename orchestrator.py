import subprocess
import select
import sys
import init_game_state
import json
import time

def start_player_script(lang, player_number):
    if lang == "cpp":
        output_binary = "player" + str(player_number) + "_out"
        subprocess.run(["g++", "player_code_runner.cpp", "player1/knight.cpp", "json.hpp", "-o", output_binary], capture_output=True)
        return subprocess.Popen(["./"+output_binary, str(player_number)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    elif lang == "py":
        return subprocess.Popen([sys.executable, "player_code_runner.py", str(player_number)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

def split_game_state_into_unit_states(game_state):
    units = game_state.get("units", [])
    unit_states = [{"units": [unit]} for unit in units]
    return unit_states

def send_unit_state(player_process, unit_state, unit_id): #FIX: print statements are breaking this function currently...
    player_process.stdin.write(unit_state + "\n")
    player_process.stdin.flush()
    output_lines = []
    action = None
    timeout = 0.05
    start_time = time.time()

    while True:
        ready_to_read, _, _ = select.select([player_process.stdout], [], [], timeout)

        if time.time() - start_time > timeout:
            action = f"{unit_id}:timeout"
            break

        if ready_to_read:
            line = player_process.stdout.readline().strip()

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
        id = unit_state.get("units")[0].get("id") #TODO: HOW DO WE MARK THE UNIT THAT IS CURRENTLY BEING PROCESSED??
        player = unit_state.get("units")[0].get("player")
        unit_state = json.dumps(unit_state)
        if(player == 0):
            actions.append(send_unit_state(player0_process, unit_state, id))
        else: 
            actions.append(send_unit_state(player1_process, unit_state, id))

    ticklog.append([tick, actions])

print(ticklog)

player0_process.terminate()
player1_process.terminate()
