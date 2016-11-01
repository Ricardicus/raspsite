#ifdef __cplusplus
extern "C" {
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "hashtable.h"
#include "snakegame.h"
#include "coffee.h"

#define BACKEND_MAX_BUFFER_SIZE			600
#define BACKEND_MAX_NBR_OF_ARGS			20
#define BACKEND_MAX_ARRAY_SIZE			100

typedef struct http_data_s {
	int * socket;
	char * client_ip;
	char * accept_time;
} http_data_t;

void error(const char*);
void write_current_coffee(int);
void output_action(int, char *);
void free_http_data(http_data_t**);
void output_index(int);
void output_path(int,const char*);
void interpret_and_output(int, char*);
void * http_callback(void *);
void print_answer(const char*, int);

#ifdef __cplusplus
}
#endif