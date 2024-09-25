CC = gcc
FLAGS = -g -Wall -Wextra -Wpedantic -DNOUTBUF -fno-strict-aliasing
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,obj/%.o,$(SRC))

all: bin/zen

bin/zen: $(OBJ)
	@mkdir -p bin
	@$(CC) $(FLAGS) -o $@ $(OBJ)

obj/%.o: src/%.c
	@mkdir -p obj
	@$(CC) $(FLAGS) -c $< -o $@

cleanobj:
	@rm -rf obj

clean:
	@rm -rf obj
	@rm -rf bin