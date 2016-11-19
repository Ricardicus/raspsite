#ifdef __cplusplus
extern "C" {
#endif 

#ifndef HTTP_H
#define HTTP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "snakegame.h"
#include "coffee.h"
#include "hashtable.h"
#include "logger.h"

#define xstr(a) 		#a
#define str(a)			xstr(a)
#define CGI_PATH		data.cgi

#define BACKEND_MAX_BUFFER_SIZE			600
#define BACKEND_MAX_NBR_OF_ARGS			20
#define BACKEND_MAX_ARRAY_SIZE			100
#define VERSION							1

typedef struct http_data_s {
	int * socket;
	char * client_ip;
	char * accept_time;
} http_data_t;

typedef void (*http_header_callback_t) (int);

void http_init(void);
void http_quit(void);
void output_index(int);
void free_http_data(http_data_t **);
void output_path(int, const char *);
void interpret_and_output(int, char *);
void * http_callback(void *);

#endif

#ifdef __cplusplus
}
#endif