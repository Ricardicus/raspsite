/*
* A Hashtable implementation
*/ 
#include "hashtable.h"

hashtable_t * new_hashtable(int size, float load)
{
	hashtable_t * hashtable = malloc(sizeof(hashtable_t));
	if ( hashtable == NULL )
		return NULL;

	hashtable->size = size;
	hashtable->ocupied = 0;
	hashtable->data_also = 0; // to be modified by user
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
	hashtable_new->data_also = hashtable->data_also;
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
	free_hashtable_table(hashtable);
	hashtable->table = hashtable_new->table;
	hashtable->size = newsize;
}

void free_hashtable(hashtable_t * hash)
{
	free_hashtable_table(hash);
	free(hash);
}

void free_hashtable_table(hashtable_t * hash)
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
			if ( hash->data_also ){
				free(ptr2->data);
				free(ptr2->key);
			}
			free(ptr2);
		}
		i++;
	}
	free(hash->table);
}

void for_each_pair(hashtable_t * hash, void (*callback)(void*,void*) )
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

			callback(ptr2->key, ptr2->data);
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

void print_table_as_chars(hashtable_t * hashtable)
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
				printf("key: %s - > data: %s\n",ptr->key,(char*)ptr->data);
				ptr=ptr->next;
			}
		}
		i++;
	}
}

void write_table_as_python_os_environ(hashtable_t * hashtable, FILE * fp)
{
	int i = 0;
	int size = hashtable->size;
	char * c;
	struct key_val_pair * ptr;

	while(i<size)
	{
		if(hashtable->table[i] != NULL)
		{
			ptr = hashtable->table[i];
			while(ptr!=NULL)
			{
				fprintf(fp,"os.environ[\"");
				c = ptr->key;
				while ( *c ){
					if ( *c != '-') {
						fputc(toupper(*c), fp);
					}
					++c;
				}
				fprintf(fp,"\"]=\"%s\"\n",(char*)ptr->data);
				ptr=ptr->next;
			}
		}
		i++;
	}
}

void write_table_as_bash_variables(hashtable_t * hashtable, FILE * fp)
{
	int i = 0;
	char *c;
	int size = hashtable->size;
	struct key_val_pair * ptr;

	while(i<size)
	{
		if(hashtable->table[i] != NULL)
		{
			ptr = hashtable->table[i];
			while(ptr!=NULL)
			{
				c = ptr->key;
				while ( *c ){
					if ( *c != '-') {
						fputc(toupper(*c), fp);
					}
					++c;
				}
				fprintf(fp,"=\"%s\"\n",(char*)ptr->data);
				ptr=ptr->next;
			}
		}
		i++;
	}
}
