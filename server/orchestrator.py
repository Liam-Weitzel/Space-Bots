import subprocess
import sys
import init_game_state
import json
import socket
import time

def start_player_script(lang, player_number):
    if lang == "cpp":
        output_binary = "player" + str(player_number) + "_out"
        subprocess.run(["sudo", "g++", "player_code_runner.cpp", "player1/knight.cpp", "json.hpp", "-o", output_binary], capture_output=True)
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

def socket_server(host, port):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen(1)
    print(f"Socket server listening on {host}:{port}")
    
    client_socket, addr = server_socket.accept()
    print(f"Client connected from {addr}")
    
    return server_socket, client_socket

def send_game_state_to_client(client_socket, game_state):
    try:
        serialized_data = json.dumps(game_state).encode('utf-8')
        client_socket.sendall(serialized_data + b'\n')
    except (BrokenPipeError, ConnectionResetError) as e:
        print(f"Connection error: {e}")
        client_socket.close()

HOST = 'localhost'
PORT = 65432
TICK_RATE = 60
TICK_INTERVAL = 1.0 / TICK_RATE

player0_process = start_player_script("py", 0)
player1_process = start_player_script("cpp", 1)

ticklog = []
game_state = init_game_state.main()
server_socket, client_socket = socket_server(HOST, PORT)

for tick in range(10000):
    start_time = time.time()
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
    send_game_state_to_client(client_socket, game_state)

    #sleep to make sure client and server are in sync
    elapsed_time = time.time() - start_time
    sleep_time = max(0, TICK_INTERVAL - elapsed_time)
    time.sleep(sleep_time)

print(ticklog)

player0_process.terminate()
player1_process.terminate()
client_socket.close()
server_socket.close()
