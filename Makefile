INPUT := tic-tac-toe.c
OUTPUT := tic-tac-toe

CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -Wshadow -O2

all:
	$(CC) $(CFLAGS) $(INPUT) -o $(OUTPUT)
