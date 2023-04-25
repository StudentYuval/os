SRC_DIR := src
OBJ_DIR := bin

EXE := $(OBJ_DIR)/hw2

SRC := $(wildcard $(SRC_DIR)/*.c)

all: $(SRC)
	mkdir -p $(OBJ_DIR)
	gcc $(SRC) -g -pthread -o $(EXE) -I include/

clean :
	rm -rf $(OBJ_DIR)
	rm count*.txt
	rm thread*.txt
	rm dispatcher.txt
	rm stats.txt
	