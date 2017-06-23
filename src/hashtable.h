#ifdef __cplusplus
extern "C" {
#endif 

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define START_SPACE		5

typedef struct key_val_pair{
	char * key;
	void * data;
	struct key_val_pair * next;
} entry_t;

typedef struct hash{
	int size;
	int ocupied;
	int data_also;
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
void free_hashtable(hashtable_t*);
void free_hashtable_table(hashtable_t*);
void* get(hashtable_t*,const char*);
int hash(hashtable_t*,const char*);
void put(hashtable_t*,char*,void*);
void print_table_callbacks(hashtable_t*);
void print_table_as_chars(hashtable_t*);
void write_table_as_python_os_environ(hashtable_t*,FILE*);
void write_table_as_bash_variables(hashtable_t*,FILE*);
void for_each_pair(hashtable_t * hash, void (*callback)(void*,void*) );

#endif

#ifdef __cplusplus
}
#endif