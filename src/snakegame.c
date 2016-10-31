#include "snakegame.h"
#include "hashtable.h"
#include <limits.h>

#define NBRINTOP	10 

static score_t ** top10 = NULL;

static unsigned long min = ULONG_MAX;

void scores_init() {
	int n;
	top10 = malloc(sizeof(score_t*) * (NBRINTOP+1));
	for(n = 0; n < NBRINTOP; ++n){
		top10[n] = NULL; 
	}
}


int score_sort_comp(const void * d1, const void * d2){
	score_t * s1, * s2;

	s1 = (score_t *) d1;
	s2 = (score_t *) d2;

	if ( d1 != NULL && d2 == NULL ){
		return 1;	
	}

	if ( d1 == NULL && d2 != NULL ){
		return -1;	
	}

	if ( d1 == NULL && d2 == NULL ){
		return 0;	
	}

	if ( s1->score < s2->score ){
		return -1;
	} else if ( s1->score == s1->score ){
		return 0;
	}

	return 1;
}


void snake_callback(int socket, hashtable_t * params){
	char * action, * name, * score, *msg;

	printf("call\n");

	action = (char*) get(params, "action");
	name = (char*) get(params, "name");
	score = (char*) get(params, "score");

	printf("action: %s\nname: %s\nscore: %s\n", action, name, score);

	if ( action == NULL ){
		return; // Nothing to do! Action MUST be specified
	}

	if ( !strcmp(action, "get_highscore") ){
		msg = "HTTP/1.0 200 OK\r\n";
   		write(socket,msg,strlen(msg));
		msg = "Content-Type: text/plain\r\n\r\n";
   		write(socket,msg,strlen(msg));

		scores_output(socket, "games/highscores/snake.txt");
	} else if ( !strcmp(action, "post_score") ){

		msg = "HTTP/1.0 200 OK\r\n";
   		write(socket,msg,strlen(msg));
		msg = "Content-Type: text/plain\r\n\r\n";
   		write(socket,msg,strlen(msg));

		if ( name == NULL || score == NULL ){
			msg = "Error: name|score NULL\r\n";
   			write(socket,msg,strlen(msg));
			return; // name and score MUST be specified
		}

		if ( top10[NBRINTOP] != NULL ) free(top10[NBRINTOP]);
		score_t * new_score = calloc(1,sizeof(score_t));

		snprintf(new_score->name, SCORES_MAX_NAME_LENGTH, "%s", name);
		new_score->score = (unsigned long) atol(score);
		top10[NBRINTOP] = new_score;

		qsort(top10,NBRINTOP+1, sizeof(score_t*), score_sort_comp);

		scores_store("games/highscores/snake.txt");

		msg = "Success\r\n";
   		write(socket,msg,strlen(msg));

	}
}

void scores_output(int socket, const char * path) {
	FILE * fp;
	int c;

	fp = fopen(path, "r");
	if ( fp == NULL ){
		return;
	}
	while ( (c=fgetc(fp)) != EOF ){
		write(socket,&c,1);
	}

	fclose(fp);
}

void scores_store(const char * path) {
	FILE * fp;
	int n = 0;

	fp = fopen(path, "w");

	for(; n < NBRINTOP && top10[n] != NULL; ++n){
		fprintf(fp, "%s\r\n%lu\r\n", top10[n]->name, top10[n]->score);
	}

	fclose(fp);
}

void scores_quit() {
	int n;

	if ( top10 == NULL )
		return;

	for(n = 0; n < NBRINTOP; ++n){
		if ( top10[n] != NULL )
			free(top10[n]); 
	}

	free(top10);
}

