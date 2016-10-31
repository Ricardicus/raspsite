CC = gcc

all: 
	$(CC) -o backend backend.c src/snakegame.c src/hashtable.c src/coffee.c
