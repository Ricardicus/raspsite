#ifdef __cplusplus
extern "C" {
#endif 

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define START_SPACE		5

typedef struct key_val_pair{
	char * key;
	void * data;
	struct key_val_pair * next;
} entry_t;

typedef struct hash{
	int size;
	int ocupied;
	float load;
	entry_t ** table;
	void (*put)(struct hash*,char*,void *);
	void (*print_table_callbacks)(struct hash*);
}	hashtable_t;

// Make sure it outputs its name on recieving the "NAME" command
typedef void *(*callback_func_t)(int,hashtable_t*, char*); 

/*
* Function declarations
*/

hashtable_t * new_hashtable(int,float);
void rehash(hashtable_t*);
void free_hashtable(hashtable_t*, int);
void free_hashtable_table(hashtable_t*, int);
void* get(hashtable_t*,const char*);
int hash(hashtable_t*,const char*);
void put(hashtable_t*,char*,void*);
void print_table_callbacks(hashtable_t*);

#endif

#ifdef __cplusplus
}
#endif