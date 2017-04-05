CC = gcc
CFLAGS = -Wall -lpthread
OBJS = backend.o snakegame.o http.o coffee.o hashtable.o notif.o tcp_file_reader.o cgi.o file_display_cgi.o http_util.o sec_session.o rsa.o

all : backend

backend: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o backend
backend.o: backend.c backend.h http.h hashtable.h snakegame.h coffee.h logger.h notif.h tcp_file_reader.h
snakegame.o: snakegame.c snakegame.h logger.h
http.o: http.c http.h snakegame.h coffee.h hashtable.h logger.h cgi.h password.h http_util.h
http_util.o: http_util.c http_util.h
coffee.o: coffee.c coffee.h
hashtable.o: hashtable.c hashtable.h
notif.o: notif.c notif.h tcp_file_reader.h
tcp_file_reader.o: tcp_file_reader.c rsa.c sec_session.c tcp_file_reader.h sec_session.h
file_display_cgi.o: file_display_cgi.c file_display_cgi.h http.h
sec_session.o: sec_session.c rsa.c sec_session.h rsa.h 
rsa.o: rsa.c rsa.h
cgi.o: cgi.c cgi.h

.PHONY : clean
clean : 
	-rm *.o $(OBJS) backend
