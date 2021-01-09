COMPILER=gcc
FLAGS=-Wall -Os -g0 -I ./include --std=c99 
LIBS=-lSDL2 -lm
SRC_DIR=./src
BIN_DIR=./bin
BIN_NAME=snake
all: 
	$(COMPILER) $(FLAGS) $(SRC_DIR)/main.c $(LIBS) -o $(BIN_DIR)/$(BIN_NAME) 
run: 
	$(BIN_DIR)/$(BIN_NAME) 
.PHONY:all
