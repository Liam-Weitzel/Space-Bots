import subprocess
import sys
import init_game_state
import json
import socket
import select
import time
import math

def start_player_script(lang, player_number):
    if lang == "cpp":
        output_binary = "player" + str(player_number) + "_out"
        subprocess.run([
            "sudo",
            "g++",
            "player_code_runner.cpp",
            f"player{player_number}/acid_ant.cpp",
            f"player{player_number}/bloated_bedbug.cpp",
            f"player{player_number}/dung_beetle.cpp",
            f"player{player_number}/engorged_tick.cpp",
            f"player{player_number}/famished_tick.cpp",
            f"player{player_number}/foraging_maggot.cpp",
            f"player{player_number}/infected_mouse.cpp",
            f"player{player_number}/lava_ant.cpp",
            f"player{player_number}/mantis.cpp",
            f"player{player_number}/mawing_beaver.cpp",
            f"player{player_number}/plague_bat.cpp",
            f"player{player_number}/rhino_beetle.cpp",
            f"player{player_number}/spider.cpp",
            f"player{player_number}/swooping_bat.cpp",
            f"player{player_number}/tainted_cockroach.cpp",
            f"player{player_number}/tunneling_mole.cpp",
            "json.hpp",
            "-o", output_binary
        ], capture_output=True)
        return subprocess.Popen(["./" + output_binary, str(player_number)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    elif lang == "py":
        return subprocess.Popen([sys.executable, "player_code_runner.py", str(player_number)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

def split_game_state_into_unit_states(game_state):
    units = game_state.get("units", [])
    terrain = game_state.get("terrain", [])
    unit_states = []

    def is_within_fov(unit1, unit2):
        # Calculate the Euclidean distance between two units
        distance = math.sqrt((unit1['position'][0] - unit2['position'][0]) ** 2 +
                             (unit1['position'][1] - unit2['position'][1]) ** 2)
        # Check if the distance is within the FOV range
        return distance <= unit1['fov']

    for current_unit in units:
        unit_dict = {
            'self': current_unit,
            'units': [],
            'terrain': terrain
        }

        # Track visible units (self and allies can extend vision)
        visible_units = set()
        to_check = [current_unit]

        while to_check:
            unit = to_check.pop()
            visible_units.add(unit['id'])

            for other_unit in units:
                # Find allied units within the FOV and extend our vision by theirs
                if other_unit['player'] == current_unit['player']:
                    if other_unit['id'] not in visible_units:
                        if is_within_fov(unit, other_unit):
                            to_check.append(other_unit)
                else: # Find enemy units within the FOV
                    if other_unit['id'] not in visible_units:
                        if is_within_fov(unit, other_unit):
                            visible_units.add(other_unit['id'])

        # Add visible units to 'units' list (excluding the current unit)
        unit_dict['units'] = [unit for unit in units if unit['id'] != current_unit['id'] and unit['id'] in visible_units]

        unit_states.append(unit_dict)

    return unit_states

def send_unit_state(player_process, unit_state, unit_id): #TODO: Add timeout for player's code, make web socket version of IPC between player's code and orchestrator for debug mode
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

def send_init_state_to_client(client_socket, player_number): #TODO: Also send terrain data to front_end this way... Implement the same concept with the player_code_runners
    try:
        data = {}
        data["player"] = player_number #NOTE: This is a horrible way to assign player numbers to clients
        serialized_data = json.dumps(data).encode('utf-8')
        client_socket.sendall(serialized_data + b'\n')
    except (BrokenPipeError, ConnectionResetError, OSError) as e:
        print(f"Connection error: {e}")
        return False  # Indicate that the client has disconnected
    return True  # Indicate that the data was sent successfully

def send_game_state_to_client(client_socket, game_state):
    try:
        serialized_data = json.dumps(game_state).encode('utf-8')
        client_socket.sendall(serialized_data + b'\n')
    except (BrokenPipeError, ConnectionResetError, OSError) as e:
        print(f"Connection error: {e}")
        return False  # Indicate that the client has disconnected
    return True  # Indicate that the data was sent successfully

def disconnect_client(client_addresses, client_sockets, client_socket):
    client_addr = client_addresses.get(client_socket, "Unknown Client")
    print(f"Removing client {client_addr} due to send failure")
    client_sockets.remove(client_socket)
    client_socket.close()
    client_addresses.pop(client_socket, None)

def socket_server(host, port):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((host, port))
    server_socket.listen(2)
    server_socket.setblocking(False)  # Set non-blocking mode
    print(f"Socket server listening on {host}:{port}")
    return server_socket

HOST = 'localhost'
PORT = 65432
TICK_RATE = 60
TICK_INTERVAL = 1.0 / TICK_RATE

# Detect player language this:
player_0_python = bool(input("player 1 python?"))
player_1_python = bool(input("player 2 python?"))
if player_0_python: 
    player0_process = start_player_script("py", 0)
else: 
    player0_process = start_player_script("cpp", 0)

if player_1_python: 
    player1_process = start_player_script("py", 1)
else: 
    player1_process = start_player_script("cpp", 1)



ticklog = []  #NOTE: Only needed for offline/ auto play, in this case also remove the sleep which sync each tick and don't send anything to front-end until match is done
game_state = init_game_state.main()
server_socket = socket_server(HOST, PORT)
client_sockets = []
client_addresses = {}
uninitialized_client_sockets = []
uninitialized_client_addresses = {}

try:
    for tick in range(10000):
        start_time = time.time()

        # Use select to check for new connections and readable sockets
        try:
            readable, _, _ = select.select([server_socket] + client_sockets, [], [], 0.05)
            #NOTE: The last param of select is how long this blocks execution for... adjust if needed
        except ValueError:
            # If any socket is invalid, remove it
            for sock in client_sockets:
                if sock.fileno() == -1:
                    client_addr = client_addresses.get(sock, "Unknown Client")
                    print(f"Client {client_addr} has disconnected.")
            client_sockets = [sock for sock in client_sockets if sock.fileno() != -1]
            continue

        for client_socket in uninitialized_client_sockets: #NOTE: Don't send game_state to clients that got connected this tick, send the init_state instead
            client_sockets.append(client_socket)
            client_addresses[client_socket] = uninitialized_client_addresses[client_socket]
            uninitialized_client_addresses[client_socket] = ''
            uninitialized_client_sockets.remove(client_socket)

        # Accept new connections
        if server_socket in readable:
            client_socket, addr = server_socket.accept()
            client_socket.setblocking(False)
            uninitialized_client_sockets.append(client_socket)
            uninitialized_client_addresses[client_socket] = addr
            print(f"Client connected from {addr}")
            if not send_init_state_to_client(client_socket, len(client_sockets)):
                disconnect_client(uninitialized_client_addresses, uninitialized_client_sockets, client_socket)

        # Handle sending game state to all connected clients
        for client_socket in client_sockets[:]:
            if client_socket in readable:
                try:
                    data = client_socket.recv(1024).decode('utf-8')
                    client_addr = client_addresses.get(client_socket, "Unknown Client")
                    if data:
                        print(f"Received data from {client_addr}: {data}")
                    else: # Client has disconnected
                        print(f"Client {client_addr} has disconnected")
                        client_sockets.remove(client_socket)
                        client_socket.close()
                        client_addresses.pop(client_socket, None)
                        continue
                except (BlockingIOError, ConnectionResetError):
                    # Ignore non-blocking read errors
                    continue

            # Send the current game state
            if not send_game_state_to_client(client_socket, game_state):
                disconnect_client(client_addresses, client_sockets, client_socket)

        unit_states = split_game_state_into_unit_states(game_state)
        actions = []

        for unit_state in unit_states:
            id = unit_state.get("self").get("id")
            player = unit_state.get("self").get("player")
            unit_state = json.dumps(unit_state)
            if player == 0:
                action = send_unit_state(player0_process, unit_state, id)
            else:
                action = send_unit_state(player1_process, unit_state, id)
            actions.append(action)

        ticklog.append([tick, actions])
        # game_state = game_engine(game_state, actions)

        # Sleep to make sure client and server are in sync
        elapsed_time = time.time() - start_time
        sleep_time = max(0, TICK_INTERVAL - elapsed_time)
        time.sleep(sleep_time)

finally:
    # Cleanup
    player0_process.terminate()
    player1_process.terminate()
    for client_socket in client_sockets:
        client_socket.close()
    server_socket.close()
