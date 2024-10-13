#gcc, flex, and bison is needed to build the executable
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -std=c99 -pedantic -pedantic-errors
SRC = $(wildcard src/*.c)
HDR = $(wildcard src/*.h)
OBJ = $(patsubst src/%.c,obj/%.o, $(SRC))

all: bin/zen

parser: src/zenLexer.c src/zenParser.c

bin/zen: $(OBJ) Makefile
	@mkdir -p bin
	@$(CC) $(CFLAGS) $(OBJ) -o bin/zen
	@bin/zen

src/zenLexer.c: parser/zenLexer.l
	@flex -o $@ $<

src/zenParser.c: parser/zenParser.y
	@bison $< -d -o $@ -Wall

obj/%.o: src/%.c $(HDR)
	@mkdir -p obj
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf obj src/zenParser.c src/zenParser.h src/zenLexer.c

cleanall: clean
	@rm -rf bin

test:
	@for t in tests/*; do \
		bin/zen $$t; \
        echo "exit status of $$t: $$?"; \
	done 

.PHONY: parser all clean cleanall test