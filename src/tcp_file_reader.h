#ifndef tcp_file_reader_h
#define tcp_file_reader_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "logger.h"
#include "notif.h"

/*
* notif.c, notif.h: 
* 	used for transefering data and other stuff 
* 	over TCP/IP	on UNIX
*/ 	

#define FILE_RECEIVE_PORT	8090

void * file_receiver_thread_callback(void *);
void * tcp_receive_file(void *);

#ifdef __cplusplus
}
#endif

#endif 
