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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "snakegame.h"
#include "http_util.h"
#include "password.h"
#include "coffee.h"
#include "hashtable.h"
#include "logger.h"
#include "cgi.h"
#include "file_display_cgi.h"

#define xstr(a) 		#a
#define str(a)			xstr(a)
#define CGI_PATH		data.cgi

#define BACKEND_MAX_BUFFER_SIZE			300
#define HTTP_MAX_BUFER_SIZE				4096
#define BACKEND_MAX_NBR_OF_ARGS			20
#define BACKEND_MAX_ARRAY_SIZE			100
#define VERSION							1
#define MAXIMUM_READ_SIZE				(1<<22)
#define BUFFER_INCREMENT				(1<<10)
#define STACK_BUFFER_SIZE	(1<<12)
#define MAX_LINE_SIZE		(512)

typedef struct http_data_s {
	int * socket;
	char * client_ip;
	char * accept_time;
} http_data_t;

typedef int (*http_header_callback_t) (int);

void http_init(void);
void http_quit(void);
void free_http_data(http_data_t **);
void interpret_and_output(int, char*, size_t);
void * http_callback(void *);
void output_path(int, const char *);
int get_next_line(char *, int, int);
void parse_http_get_headers_and_arguments(hashtable_t*, char*, size_t);
void parse_http_post_data(hashtable_t *, char *);
void parse_http_headers(hashtable_t *, char *);
void urldecode2(char *, const char *);

/* HTTP-Headers */
void output_file_not_found(int);
int output_internal_server_error(int);
int output_html_headers(int);
int output_css_headers(int);
int output_jpg_headers(int);
int output_json_headers(int);
int output_woff2_headers(int);
void output_file_transfer_headers(int,char*);
int output_js_headers(int);
int output_txt_headers(int);

#endif

#ifdef __cplusplus
}
#endif