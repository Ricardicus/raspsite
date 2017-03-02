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
#include <pthread.h>
#include <stdint.h>
#include "snakegame.h"
#include "coffee.h"
#include "hashtable.h"
#include "logger.h"
#include "cgi.h"

#define xstr(a) 		#a
#define str(a)			xstr(a)
#define CGI_PATH		data.cgi

#define BACKEND_MAX_BUFFER_SIZE			300
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
void free_http_data(http_data_t **);
void interpret_and_output(int, char*);
void * http_callback(void *);
void output_path(int, const char *);
void output_index(int);
int get_next_line(char *, int, int);
void parse_http_get_headers_and_arguments(hashtable_t*, char*);
void parse_http_post_data(hashtable_t *, char *);
void parse_http_headers(hashtable_t *, char *);

/* HTTP-Headers */
void output_file_not_found(int);
void output_html_headers(int);
void output_css_headers(int);
void output_jpg_headers(int);
void output_json_headers(int);
void output_js_headers(int);
void output_txt_headers(int);

#endif

#ifdef __cplusplus
}
#endif