#ifdef __cplusplus
extern "C" {
#endif 

#ifndef SNAKEGAME_H
#define SNAKEGAME_H	

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "logger.h"	

#define SCORES_MAX_NAME_LENGTH		30

#define VERSION						1

typedef struct score_t score_t;

struct score_t {
	char name[SCORES_MAX_NAME_LENGTH+1];
	unsigned long score;
};

void scores_init(void);
void scores_print(void);
void scores_output(int, const char*);
void scores_store(const char*);
void scores_quit(void);

void snake_callback(int, hashtable_t *);

#endif

#ifdef __cplusplus
}
#endif