#ifdef __cplusplus
extern "C" {
#endif

#ifndef SEC_SESSION_H
#define SEC_SESSION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "rsa.h"

#define INITIALIZE		1
#define CONVERSATE		2 

void sec_session(int);

#endif

#ifdef __cplusplus
}
#endif