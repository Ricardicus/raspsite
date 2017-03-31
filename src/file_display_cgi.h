#ifdef __cplusplus
extern "C" {
#endif 

#ifndef FILE_DISPLAY_H
#define FILE_DISPLAY_H

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
#include "http.h"
#include "snakegame.h"
#include "coffee.h"
#include "hashtable.h"
#include "logger.h"
#include "cgi.h"

void file_display_cgi(int, hashtable_t*);

#endif

#ifdef __cplusplus
}
#endif