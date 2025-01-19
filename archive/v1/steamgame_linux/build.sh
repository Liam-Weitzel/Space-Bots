#!/bin/bash

# Navigate to the library directory
cd library

# Compile the code
g++ -std=c++11 -o SteamworksPy.so -shared -fPIC SteamworksPy.cpp -l steam_api -L./sdk/redist

# Check if compilation was successful
if [ $? -eq 0 ]; then
    # Move the output file back to parent directory
    mv SteamworksPy.so ../
    echo "Compilation successful. SteamworksPy.so moved to parent directory."
else
    echo "Compilation failed."
    exit 1
fi
