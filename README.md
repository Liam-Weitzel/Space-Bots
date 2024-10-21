# Gamefied-Code-Dev

orchestrator:
-input: 2 scripts, initial game state
-output: all games states, match results in auto mode
-output: game_state sent to clients every tick through websockets, in live mode
-spawn two child processes (player_code_runner), and runs player's functions on iterative game states until game concludes.
-calls game_engine
-calls game_state_init

game_state_init:
-input: player data, game settings
-output: initial game state
-game state columns: HP, Location, Shape, Size, Orientation, Type, Previous action, Previous death, Remaining Cooldowns, Lockout

game_engine:
-Input: game state, player actions
-Output: next game state

player_code_runner:
-Runs code from user file

client:
-input: next game state
-output: visuals
-listens for user inputs, sends it back to orchestrator

Later:
-Web app
-GUI
-Database

TODO:
- Refactor code with return types and type defs
- Add asserts everywhere for simulation testing
- Create some unit tests
- Detect which player the client is, reconnecting as the same player

- fix web socket crashing when one mis read!!

- Create function that reads tile map jsons and blits on screen
- When blitting on screen, use the layers to determine the order (all at once at the end of the draw loop)
