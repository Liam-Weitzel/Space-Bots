import subprocess
import sys
import init_game_state
import json
import socket
import select
import time

def start_player_script(lang, player_number):
    if lang == "cpp":
        output_binary = "player" + str(player_number) + "_out"
        subprocess.run([
            "g++",
            "player_code_runner.cpp",
            "player1/acid_ant.cpp",
            "player1/bloated_bedbug.cpp",
            "player1/dung_beetle.cpp",
            "player1/engorged_tick.cpp",
            "player1/famished_tick.cpp",
            "player1/foraging_maggot.cpp",
            "player1/infected_mouse.cpp",
            "player1/lava_ant.cpp",
            "player1/mantis.cpp",
            "player1/mawing_beaver.cpp",
            "player1/plague_bat.cpp",
            "player1/rhino_beetle.cpp",
            "player1/swooping_bat.cpp",
            "player1/tainted_cockroach.cpp",
            "player1/tunneling_mole.cpp",
            "json.hpp",
            "-o", output_binary
        ], capture_output=True)
        return subprocess.Popen(["./" + output_binary, str(player_number)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
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

def send_game_state_to_client(client_socket, game_state):
    try:
        serialized_data = json.dumps(game_state).encode('utf-8')
        client_socket.sendall(serialized_data + b'\n')
    except (BrokenPipeError, ConnectionResetError, OSError) as e:
        print(f"Connection error: {e}")
        return False  # Indicate that the client has disconnected
    return True  # Indicate that the data was sent successfully

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

player0_process = start_player_script("py", 0)
player1_process = start_player_script("cpp", 1)

ticklog = []  #NOTE: Only needed for offline play
game_state = init_game_state.main()
server_socket = socket_server(HOST, PORT)
client_sockets = []
client_addresses = {}

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

        # Accept new connections
        if server_socket in readable:
            client_socket, addr = server_socket.accept()
            client_socket.setblocking(False)
            client_sockets.append(client_socket)
            client_addresses[client_socket] = addr
            print(f"Client connected from {addr}")

        # Handle sending game state to all connected clients
        for client_socket in client_sockets[:]:
            if client_socket in readable:
                try:  # Try to receive data (to detect disconnection)
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
                # If sending fails, remove the socket and close it
                client_addr = client_addresses.get(client_socket, "Unknown Client")
                print(f"Removing client {client_addr} due to send failure")
                client_sockets.remove(client_socket)
                client_socket.close()
                client_addresses.pop(client_socket, None)

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
