#ifdef __cplusplus
extern "C" {
#endif 

#ifndef HTTP_H
#define HTTP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "snakegame.h"
#include "coffee.h"

#define xstr(a) 		#a
#define str(a)			xstr(a)
#define CGI_PATH		data.cgi

#define BACKEND_MAX_BUFFER_SIZE			600
#define BACKEND_MAX_NBR_OF_ARGS			20
#define BACKEND_MAX_ARRAY_SIZE			100
#define VERSION							1

#define	log_error(...)	do { printf("error:%s.%s:%d ", __FILE__, __func__, __LINE__); FILE * fp_log = fopen("log/logg.txt", "a"); fprintf(fp_log, __VA_ARGS__); printf(__VA_ARGS__); fclose(fp_log); } while(0)
#define log(...)		do { FILE * fp_log = fopen("log/logg.txt", "a"); fprintf(fp_log, __VA_ARGS__); printf(__VA_ARGS__); fclose(fp_log); } while(0)

typedef struct http_data_s {
	int * socket;
	char * client_ip;
	char * accept_time;
} http_data_t;

void output_index(int);
void free_http_data(http_data_t **);
void output_path(int, const char *);
void interpret_and_output(int, char *);
void * http_callback(void *);

#endif

#ifdef __cplusplus
}
#endif