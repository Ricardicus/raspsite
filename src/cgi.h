/*
* cgi.c/.h are written to make it possible to write cgi's
* for the server in other languages than C.
*/ 

#ifdef __cplusplus
extern "C" {
#endif 

#ifndef CGI_H
#define CGI_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "hashtable.h"
#include "logger.h"

void	cgi_py(int, hashtable_t*, const char*);
void	cgi_sh(int, hashtable_t*, const char*);
char*	traverse_path_to_find(const char*);

#endif

#ifdef __cplusplus
}
#endif 