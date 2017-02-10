/*
* A Hashtable implementation
*/ 
#include "hashtable.h"

hashtable_t * new_hashtable(int size, float load)
{
	hashtable_t * hashtable = malloc(sizeof(hashtable_t));
	hashtable->size = size;
	hashtable->ocupied = 0;
	hashtable->load = load;
	hashtable->table = (entry_t**) calloc(size,sizeof(entry_t*));
	hashtable->put = put;
	hashtable->print_table_callbacks = print_table_callbacks;
	return hashtable;
}

void rehash(hashtable_t * hashtable)
{
	int size_old = hashtable->size;
	int newsize = size_old * 2;
	hashtable_t * hashtable_new = new_hashtable(newsize,hashtable->load);
	int i = 0;
	while(i<size_old)
	{
		struct key_val_pair * ptr = hashtable->table[i];
		while(ptr!=NULL)
		{
			hashtable_new->put(hashtable_new,ptr->key,ptr->data);
			ptr = ptr->next;
		}
		i++;
	}
	free_hashtable_table(hashtable,0);
	hashtable->table = hashtable_new->table;
	hashtable->size = newsize;
}

void free_hashtable(hashtable_t * hash, int data_also)
{
	free_hashtable_table(hash,data_also);
	free(hash);
}

void free_hashtable_table(hashtable_t * hash, int data_also)
{
	int size = hash->size;
	int i = 0;
	struct key_val_pair * ptr1;
	struct key_val_pair * ptr2;
	while(i<size)
	{
		ptr1 = hash->table[i];
		while(ptr1!=NULL)
		{
			ptr2 = ptr1;
			ptr1 = ptr1->next;
			if ( data_also )
				free(ptr2->data);
			free(ptr2);
		}
		i++;
	}
	free(hash->table);
}

int hash(hashtable_t * hashtable,const char * str)
{	
	int size = hashtable->size;
	int hash = 0;
	while(*str)
	{
		hash+=*str * *str;
		str++;
	}
	if(size == 0) return 0;
	return hash % size;
}

void put(hashtable_t * hashtable, char * key, void * val)
{
	float load_level = (float) hashtable->ocupied * hashtable->load;
	float max_load_level = (float) hashtable->size * hashtable->load;
	if(load_level > max_load_level) 
	{
		rehash(hashtable);
		hashtable->put(hashtable,key,val);
		return;
	} 
	int index = hash(hashtable,key);
	if(hashtable->table[index] == NULL)
	{
		hashtable->table[index] = malloc(sizeof(entry_t));
		hashtable->table[index]->key = key;
		hashtable->table[index]->data = val;
		hashtable->table[index]->next = NULL;
		hashtable->ocupied++;
		return;
	} 
	else 
	{
		struct key_val_pair * ptr1 = hashtable->table[index];
		struct key_val_pair * ptr2;
		while(ptr1 != NULL)
		{
			if(strcmp(ptr1->key,key) == 0)
			{
				ptr1->data = val;
				return;
			}
			ptr2 = ptr1;
			ptr1 = ptr1->next;
		}
		ptr2->next = malloc(sizeof(entry_t));
		ptr2 = ptr2->next;
		ptr2->key = key;
		ptr2->data = val;
		ptr2->next = NULL;
		hashtable->ocupied++;
	}
}

void * get(hashtable_t * hashtable, const char * key)
{
	int index = hash(hashtable,key);
	if(hashtable->table[index] == NULL)
	{
		return NULL;
	}
	struct key_val_pair * p = hashtable->table[index];
	while(p!=NULL)
	{	
		if(strcmp(p->key,key) == 0)
		{
			return p->data;
		}
		p = p->next;
	}
	return NULL;
}

void print_table_callbacks(hashtable_t * hashtable)
{
	int i = 0;
	int size = hashtable->size;
	struct key_val_pair * ptr;

	while(i<size)
	{
		if(hashtable->table[i] != NULL)
		{
			ptr = hashtable->table[i];
			while(ptr!=NULL)
			{
				printf("key: %s - > data: %s\n",ptr->key,(char*)(((callback_func_t) ptr->data))(0,NULL,"NAME"));
				ptr=ptr->next;
			}
		}
		i++;
	}
}