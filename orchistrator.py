import subprocess
import os

# Compiles a C++ file into an executable.
def compile_cpp_script(cpp_file, output_file):
    compile_process = subprocess.run(['g++', cpp_file, '-o', output_file], capture_output=True)

    if compile_process.returncode != 0:
        print(f"Error compiling C++ file {cpp_file}:", compile_process.stderr.decode())
        exit(1)

def start_player_script(script_path):
    file_ext = os.path.splitext(script_path)[1]

    if file_ext == ".cpp":
        # Compile the C++ script into an executable
        output_binary = os.path.splitext(script_path)[0]  # Binary without the .cpp extension
        compile_cpp_script(script_path, output_binary)
        
        # Return the process of the compiled C++ binary
        return subprocess.Popen(["./"+output_binary], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    
    elif file_ext == ".py":
        # Return the process of the Python script
        return subprocess.Popen(["python", script_path], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    
    else:
        raise ValueError(f"Unsupported file type: {file_ext}")

def send_game_state(player_process, game_state):
    # Send the game state to the player process and read the action from stdout.
    player_process.stdin.write(game_state + "\n")
    player_process.stdin.flush()
    return player_process.stdout.readline().strip()

def stop_process(player_process, stop_signal="STOP"):
    # Send stop signal to player process and terminate it.
    try:
        player_process.stdin.write(stop_signal + "\n")
        player_process.stdin.flush()
        player_process.terminate()
    except Exception as e:
        print(f"Failed to stop process: {e}")

# Start both player scripts (C++ and Python)
player1_process = start_player_script("player1_script.py")  # Python file
player2_process = start_player_script("player2_script.cpp")  # C++ file

# Game loop (persistent processes, no respawn)
ticklog = []
for tick in range(1000):
    game_state = f"{tick}"

    # Send game state to both players and get their actions
    action_player1 = send_game_state(player1_process, game_state)
    action_player2 = send_game_state(player2_process, game_state)

    # Log the actions for this tick
    ticklog.append({"tick": tick, "player1": action_player1, "player2": action_player2})

    # Print or process actions as required
    print(f"Tick {tick}: Player 1 -> {action_player1}, Player 2 -> {action_player2}")

# Stop both processes after the game ends
stop_process(player1_process)
stop_process(player2_process)
