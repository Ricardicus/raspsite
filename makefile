CC = gcc
FLAGS = -Wall

all: 
	$(CC) $(FLAGS) -o backend src/backend.c src/snakegame.c src/http.c src/hashtable.c src/coffee.c
