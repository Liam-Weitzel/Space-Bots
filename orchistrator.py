import subprocess
import os

game_state = "character_position: 5, health: 100"

def run_py(path):
    process = subprocess.Popen(["python", path], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout, _ = process.communicate(input=game_state.encode())
    return stdout.decode().strip()

def compile_cpp(path, out):
    compile_process = subprocess.run(['g++', path, '-o', out], capture_output=True)

    if compile_process.returncode != 0:
        print("Error compiling C++ file:", compile_process.stderr.decode())
        exit(1)

cpp_file = "player2_script.cpp"
compiled_binary = "./player2_script"
compile_cpp(cpp_file, compiled_binary)

def run_cpp(path):
    process = subprocess.Popen([path], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout, _ = process.communicate(input=game_state.encode())
    return stdout.decode().strip()

for currTick in range(1000):
    action_player1 = run_py("./player1_script.py");
    print(f"Player 1 action: {action_player1}")

    action_player2 = run_cpp(compiled_binary)
    print(f"Player 2 action: {action_player2}")
    pass
