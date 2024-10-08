import subprocess
import os

# Compiles a C++ file into an executable.
def compile_cpp(cpp_file, output_file):
    compile_process = subprocess.run(['g++', cpp_file, '-o', output_file], capture_output=True)
    if compile_process.returncode != 0:
        print(f"Error compiling C++ file {cpp_file}:", compile_process.stderr.decode())
        exit(1)

def start_player_script(lang, player_number):
    if lang == "cpp":
        # Compile the C++ script into an executable
        output_binary = "player" + str(player_number) + "_out"
        compile_cpp("player_code_runner.cpp", output_binary)

        # Return the process of the compiled C++ binary
        return subprocess.Popen(["./"+output_binary, str(player_number)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    elif lang == "py":
        # Return the process of the Python script
        return subprocess.Popen(["python", "player_code_runner.py", str(player_number)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

def send_game_state(player_process, game_state):
    # Send the game state to the player process and read the action from stdout.
    player_process.stdin.write(game_state + "\n")
    player_process.stdin.flush()
    #TODO: Wait for action to come out, pass back print statements seperately
    return player_process.stdout.readline().strip()

def stop_process(player_process, stop_signal="STOP"):
    try: # Send stop signal to player process and terminate it.
        player_process.stdin.write(stop_signal + "\n")
        player_process.stdin.flush()
        player_process.terminate()
    except Exception as e:
        print(f"Failed to stop process: {e}")

# Start both player scripts (C++ and Python)
player1_process = start_player_script("py", 1)
player2_process = start_player_script("cpp", 2)

ticklog = []
for tick in range(10000):
    game_state = f"{tick}"

    #TODO: each run should be timed, and killed if it takes too long...
    #      parse out the action, all other print statements should be
    #      passed back to front-end in order for the user to debug.
    #      If the users code becomes very complex and hard to run,
    #      at some point it will be faster to multithread. For now, not

    # Send game state to both players and get their actions
    player1_action = send_game_state(player1_process, game_state)
    player2_action = send_game_state(player2_process, game_state)

    # Log the actions for this tick
    ticklog.append({"tick": tick, "player1": player1_action, "player2": player2_action})

print(ticklog)

# Stop both processes after the game ends
stop_process(player1_process)
stop_process(player2_process)
