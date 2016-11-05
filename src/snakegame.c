#include "snakegame.h"
#include "hashtable.h"
#include <limits.h>

#define NBRINTOP		10 
#define MAXLINELENGTH	50

static score_t ** top10 = NULL;

static unsigned long min = ULONG_MAX;

void scores_init() {
	int n,q;
	FILE * fp;
	char line[MAXLINELENGTH+1], *c, *l;
	memset(line,'\0', sizeof(line));

	top10 = calloc(NBRINTOP+1,sizeof(score_t*));

	fp = fopen("games/highscores/snake.txt", "r");
	n = q = 0;
	while ( fgets(line, MAXLINELENGTH, fp) != NULL ){
		if (top10[q] == NULL ) top10[q] = calloc(1,sizeof(score_t));
		c = l = line;
		if ( *c == '\n')
			l++;
		c++;
		while(*c){
			if ( *c == '\n' ){
				*c = '\0';
			}
			c++;
		} 

		if ( !(n % 2) ) {
			snprintf(top10[q]->name, SCORES_MAX_NAME_LENGTH, "%s", l); 
		} else {
			top10[q]->score = (unsigned long) atol(line);
			++q;
		}

		++n;
		memset(line, '\0', sizeof(line));
	}


	for(n = q; n < NBRINTOP+1; ++n){
		top10[n] = NULL;
	}

}


int score_sort_comp(const void * d1, const void * d2){
	const score_t * s1, * s2;

	s1 = *(const score_t **) d1;
	s2 = *(const score_t **) d2;

	if ( s1 == NULL && s2 != NULL ){
		return 1;
	} else if ( s1 != NULL && s2 == NULL){
		return -1;
	} else if ( s2 == NULL && s1 == NULL ){
		return 0;
	}

	if ( s1->score > s2->score ){
		return -1;
	} else if ( s1->score == s2->score ){
		return 0;
	}

	return 1;
}


void snake_callback(int socket, hashtable_t * params){
	char * action, * name, * score, *msg;
	int i, update = 0;
	unsigned long new_score;

	action = (char*) get(params, "action");
	name = (char*) get(params, "name");
	score = (char*) get(params, "score");

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
		new_score = atol(score);

		update = i = 0;
		for (; i < NBRINTOP && !update; i++){
			if ( top10[i] == NULL)
				continue;
			if ( !strcmp(top10[i]->name, name) ){
				if ( top10[i]->score < new_score ){
					top10[i]->score = new_score;
				}
				update = 1;
			}
		}

		if ( ! update ){
			if ( top10[NBRINTOP] != NULL ) free(top10[NBRINTOP]);
			score_t * new_score_t = calloc(1,sizeof(score_t));

			snprintf(new_score_t->name, SCORES_MAX_NAME_LENGTH, "%s", name);
			new_score_t->score = new_score;
			top10[NBRINTOP] = new_score_t;
		}

		qsort(top10,NBRINTOP+1, sizeof(score_t**), score_sort_comp);

		scores_print();

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

void scores_print(){
	int i = 0;

	if ( top10 == NULL )
		return;

	printf("%s\n",__func__);
	for (; i<NBRINTOP; ++i){
		if ( top10[i] == NULL ){
			printf("p. %d : NULL\n",i);
		} else {
			printf("p. %d : %s (%lu)\n",i,top10[i]->name, top10[i]->score);
		}
	}


}

void scores_store(const char * path) {
	FILE * fp;
	int n = 0;

	fp = fopen(path, "w");

	for(; n < NBRINTOP && top10[n] != NULL; ++n){
		fprintf(fp, "%s\n%lu\n", top10[n]->name, top10[n]->score);
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

