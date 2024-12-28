RAYLIB ?= ./external/raylib/src/
EXTERNAL ?= ./external/
SRC ?= ./src/

all:
	g++ src/main.cpp -I $(RAYLIB) -I $(EXTERNAL) -I $(SRC) -L $(RAYLIB) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
