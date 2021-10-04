CC = gcc
CFLAGS = -g -std=c99 -pedantic -Wall -fsanitize=address -I./include

SRC = $(wildcard ./src/*.c)
OBJS = $(patsubst %.c, %.o, $(SRC))

.PHONY: all build clean

all: clean build

build: ./bin/main


main.o: main.c
	$(CC) $(CFLAGS) -c $^ -o $@

./src/%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@ -lm

./bin/main: main.o $(OBJS)
	$(CC) $(CFLAGS) -lm -o $@ $^ -lm

clean:
	rm -rf $(OBJS) main.o ./bin/main
