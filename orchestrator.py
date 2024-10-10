import subprocess
import os
import select

def start_player_script(lang, player_number):
    if lang == "cpp":
        output_binary = "player" + str(player_number) + "_out"
        compile_process = subprocess.run(["g++", "player_code_runner.cpp", "player2\/knight.cpp" "-o", output_binary], capture_output=True)
        return subprocess.Popen(["./"+output_binary, str(player_number)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    elif lang == "py":
        return subprocess.Popen(["python", "player_code_runner.py", str(player_number)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

def split_game_state_into_unit_states(game_state):
    return [game_state, game_state, game_state] #TODO: implement

def send_unit_state(player_process, unit_state, unit_id):
    player_process.stdin.write(unit_state + "\n")
    player_process.stdin.flush()
    output_lines = []
    action = None
    timeout = 0.01

    while True:
        ready_to_read, _, _ = select.select([player_process.stdout], [], [], timeout)

        if ready_to_read:
            line = player_process.stdout.readline().strip()
            
            if line.startswith(f"{unit_id}:"):
                action = line
                break
            else:
                output_lines.append(line)
        else:
            action = f"{unit_id}:timeout"
            break

    return action, output_lines

# Start both player scripts (C++ and Python)
player1_process = start_player_script("py", 1)
player2_process = start_player_script("cpp", 2)

ticklog = []
for tick in range(10000):
    game_state = "1"
    unit_states = split_game_state_into_unit_states(game_state)

    for unit_state in unit_states:
        player1_action = send_unit_state(player1_process, unit_state, 1)
        player2_action = send_unit_state(player2_process, unit_state, 1)

    # Log the actions for this tick
    ticklog.append([tick, player1_action, player2_action])

print(ticklog)

# Stop both processes after the game ends
player1_process.terminate()
player2_process.terminate()
