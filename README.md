# Gamefied-Code-Dev

TODO:
Game state init:
-input: player data, game settings
-output: initial game state (dictionairy of dataframe)
-game state columns: HP, Location, Shape, Size, Orientation, Type, Previous action, Previous death, Remaining Cooldowns, Lockout

Write orchestrator:
-input: 2 scripts, initial game state
-output: all games states, match results in auto mode
-output: visualization in live mode
-computes all game logic
-initializes two child processes, and runs input functions on iterative game states until game concludes.

Game engine:
-Input: game state, player actions (auto or live)
-Output: next game state

Renderer:
-input: next game state
-output: visuals

Player code runner:
-Runs code from user file

Player controlled:
-Listens for user inputs

User file:
-Prototype (done)

Later:
-Web app
-GUI
-Database
