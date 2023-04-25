SRC_DIR := src

EXE := hw5

SRC := $(wildcard $(SRC_DIR)/*.c)

all: $(SRC)
	gcc $(SRC) -g -pthread -o $(EXE) -I include/

clean :
	rm -rf $(EXE)