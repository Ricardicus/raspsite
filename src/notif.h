#ifndef notif_h
#define notif_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "logger.h"

/*
* notif.c, notif.h: 
* 	used for transefering data and other stuff 
* 	over TCP/IP	on UNIX
*/ 	

// Macros for communication commands
#define SEND_FILE			1
#define SEC_SESSION			2

// Macros for receiving
#define EVERYTHING_OK		1
#define TIMEOUT				2
#define SOMETHING_WRONG		3
#define RECEIVED_OVERWRITE	4

int 	make_contact(const char *, int);
void 	output_greeting(int);
void 	output_alternatives(int);
int 	output_file(int, const char *, char *);
void 	receive_file(int, const char*);

#ifdef __cplusplus
}
#endif

#endif 

