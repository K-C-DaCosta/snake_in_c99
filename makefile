COMPILER=gcc
FLAGS=-Wall -O0 -g3 -I ./include --std=c99 
LIBS=-lSDL2 -lm
SRC_DIR=./src
BIN_DIR=./bin
BIN_NAME=snake
all: 
	$(COMPILER) $(FLAGS) $(SRC_DIR)/main.c $(LIBS) -o $(BIN_DIR)/$(BIN_NAME) 
run: 
	$(BIN_DIR)/$(BIN_NAME) 
.PHONY:all