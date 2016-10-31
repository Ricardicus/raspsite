#ifdef __cplusplus
extern "C" {
#endif 

#ifndef HASHTABLE_H
#include "hashtable.h"
#endif

#include <unistd.h>
#define SCORES_MAX_NAME_LENGTH		30

typedef struct score_t score_t;

struct score_t {
	char name[SCORES_MAX_NAME_LENGTH+1];
	unsigned long score;
};

void scores_init();
void scores_output(int, const char*);
void scores_store();
void scores_quit();

void snake_callback(int, hashtable_t *);


#ifdef __cplusplus
}
#endif