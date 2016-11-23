CC = gcc
FLAGS = -Wall
OBJS = backend.o snakegame.o http.o coffee.o hashtable.o

all : backend

backend: $(OBJS)
backend.o: backend.c backend.h http.h hashtable.h snakegame.h coffee.h logger.h
snakegame.o: snakegame.c snakegame.h logger.h
http.o: http.c http.h snakegame.h coffee.h hashtable.h logger.h
coffee.o: coffee.c coffee.h
hashtable.o: hashtable.c hashtable.h

.PHONY : clean
clean : 
	-rm *.o $(OBJS) backend