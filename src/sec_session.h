#ifdef __cplusplus
extern "C" {
#endif

#ifndef SEC_SESSION_H
#define SEC_SESSION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <inttypes.h>
#include "rsa.h"
#include "notif.h"

#define INITIALIZE		1
#define CONVERSATE		2 

#define MAX_RESPONSES 		100
#define MAX_SYM_KEY_LEN		255
#define MAX_DATA_LEN		255
#define MAX_MSG_LEN			199
#define STNDRD_SYM_KEY_LEN	8


typedef enum response_alternatives_t {
	GREET = 0,
	ALTERNATIVES = 1,
	MISUNDERSTOOD = 2,
} response_alternatives_t;

void sec_session_server(int);
void sec_session_client(int);

#endif

#ifdef __cplusplus
}
#endif