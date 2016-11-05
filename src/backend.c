/* Server in the internet domain using TCP
   Port passed as an argument */
#include "backend.h"
#include <stdbool.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

static volatile bool run_this_server_please_mister = true;
static hashtable_t * dict;

void * input_reader_callback(void * data)
{
	int c;
	bool keep_going = true;
	while ( ((c=fgetc(stdin)) != EOF) && keep_going){
		if ( c == (int) 'q' )
			keep_going = false;
	}

	run_this_server_please_mister = false;
	return NULL;
}

void output_action(int socket, char * action)
{

	char * msg = "HTTP/1.0 200 OK\r\n";
   	write(socket,msg,strlen(msg));
   	msg = "Cache-Control: no-cache, no-store, must-revalidate\r\n";
   	write(socket,msg, strlen(msg));
   	msg = "Pragma: no-cache\r\n";
	write(socket,msg, strlen(msg));
	msg = "Expires: 0\r\n";
	write(socket,msg, strlen(msg));
	msg = "Content-Type: text/plain; charset=utf-8\r\n\r\n";
   	write(socket,msg, strlen(msg));

	if (!strcmp(action, "current_coffee")){
		write_current_coffe(socket);
	}

}

void output_index(int socket)
{
	char * msg = "HTTP/1.0 200 OK\r\n";
   	write(socket,msg,strlen(msg));
   	msg = "Cache-Control: no-cache, no-store, must-revalidate\r\n";
   	write(socket,msg, strlen(msg));
   	msg = "Pragma: no-cache\r\n";
	write(socket,msg, strlen(msg));
	msg = "Expires: 0\r\n";
	write(socket,msg, strlen(msg));
   	msg = "Content-Type: text/html; charset=utf-8\r\n\r\n";
   	write(socket,msg, strlen(msg));

   	FILE * fp = fopen("index.html", "r");
   	int c;
   	while ( (c = fgetc(fp)) != EOF ){
   		write(socket, &c, 1);
   	}
   	fclose(fp);

   	close(socket);
}

void free_http_data(http_data_t ** http_data)
{
	free((*http_data)->client_ip);
	free((*http_data)->accept_time);
	free((*http_data)->socket);
	free((*http_data));
	*http_data = NULL;
}

void output_path(int socket, const char * path)
{
	char * msg = "HTTP/1.1 200 OK\r\n";
   	write(socket,msg,strlen(msg));

	FILE * fp = fopen(path, "r");
	if ( ! fp ){
		// sending a signal that it is no file with that name present.
		log("error: file type of requested resource %s was not found.\n", path);
		msg = "Content-Type: application/json; charset=utf-8\r\n\r\n";
   		write(socket,msg, strlen(msg));

   		msg = "{ \"error\":\"could not find requested resource.\" } ";
   		write(socket,msg, strlen(msg));
		return;
	}
	// file existed!;

	if ( ! memcmp(path, str(CGI_PATH), strlen(str(CGI_PATH))) ){
		// cgi call

		


	}

	const char * file_type = path;
	while (*file_type && *file_type != '.'){
		file_type++;
	}

	if ( !*file_type ){
		// if the file type_was not found.. 
		log("error: file type of requested resource %s was not found.\n", path);
		msg = "Content-Type: text/html; charset=utf-8\r\n\r\n";
   		write(socket,msg, strlen(msg));
		msg = "<!DOCTYPE html><body><pre>";
   		write(socket,msg, strlen(msg));
   		int c;
   		while ( (c = fgetc(fp)) != EOF ){
   			write(socket, &c, 1);
   		}
   		fclose(fp);
   		msg = "</pre></body></html>";
   		write(socket,msg, strlen(msg));
   		close(socket);
   		return;
	}

	file_type++;

	if ( !strcmp(file_type, "css") ){
		// CSS requested!
		msg = "Content-Type: text/css; charset=utf-8\r\n\r\n";
   		write(socket,msg, strlen(msg));
   		int c;
   		while ( (c = fgetc(fp)) != EOF ){
   			write(socket, &c, 1);
   		}
   		fclose(fp);
   		close(socket);
   		return;
	}

	if ( !strcmp(file_type, "html") ){
		// CSS requested!
		msg = "Content-Type: text/html; charset=utf-8\r\n\r\n";
   		write(socket,msg, strlen(msg));
   		int c;
   		while ( (c = fgetc(fp)) != EOF ){
   			write(socket, &c, 1);
   		}
   		fclose(fp);
   		close(socket);
   		return;
	} 


	if ( !strcmp(file_type, "jpg") || !strcmp(file_type, "jpeg") || !strcmp(file_type, "gif") ){
		// Image requested!
		msg = "Content-Description: File Transfer\r\n";
   		write(socket,msg, strlen(msg));	
   		msg = "Content-Transfer-Encoding: binary\r\n";
   		write(socket,msg, strlen(msg));	
   		msg = "Content-Disposition: attachment; filename=\"image.jpg\"\r\n";
   		write(socket,msg, strlen(msg));	
		msg = "Content-Type: image/jpeg\r\n\r\n";
   		write(socket,msg, strlen(msg));
   		int c;
   		while ( (c = fgetc(fp)) != EOF ){
   			write(socket, &c, 1);
   		}
   		fclose(fp);
   		close(socket);
   		return;
	}

	if ( !strcmp(file_type, "js") ){
		// Script requested!!
		msg = "Content-Type: application/javascript; charset=utf-8\r\n\r\n";
   		write(socket,msg, strlen(msg));
   		int c;
   		while ( (c = fgetc(fp)) != EOF ){
   			write(socket, &c, 1);
   		}
   		fclose(fp);
   		close(socket);
   		return;
	}

	// If nothing mapped... Default output!!!!!!
	msg = "Content-Type: text/html; charset=utf-8\r\n\r\n";
	write(socket,msg, strlen(msg));
	msg = "<!DOCTYPE html><body><pre>";
	write(socket,msg, strlen(msg));
	int c;
	while ( (c = fgetc(fp)) != EOF ){
		write(socket, &c, 1);
	}
	fclose(fp);
	msg = "</pre></body></html>";
	write(socket,msg, strlen(msg));
	close(socket);
	return;
}

void interpret_and_output(int socket, char * first_line)
{

	char *c, *args, *command, *path, *cleaner;
	int arg_count,i;
	hashtable_t * params;

	// Getting the HTTP command and path!
	c = first_line;
	path = NULL;

	while (*c){
		if ( *c == ' '){
			*c = '\0';
			if ( path == NULL ) {
				path = &c[1]; 
			}
		}
		++c;
	}
	command = first_line;

	if ( !strcmp(command, "GET") ){
		// We have recieved a 'GET' request!

		if ( !strcmp(path, "/") ){
 			// Outputting the index file!
			output_index(socket);
   			return;
		}

		if ( strstr(path, "coffe.cgi") != NULL ){
			/*
			* the coffe.cgi. Args: action
			*/ 
			arg_count = 0;
		 	params = new_hashtable(BACKEND_MAX_NBR_OF_ARGS, 0.8);

			args = strstr(path, "action=");
			cleaner = path;
			while ( *cleaner ){
				if ( *cleaner == '&'){
					*cleaner = '\0';
				}
				++cleaner;
			}

			output_action(socket, args + 7);

			free_hashtable(params);
		} else if ( strstr(path, "game.cgi") != NULL ){
			/*
			* the game.cgi, args: action=[post_score|get_highscore], name=[*], score=[*]
			*/ 

			arg_count = 0;
			params = new_hashtable(BACKEND_MAX_NBR_OF_ARGS, 0.8);

			args = strstr(path, "action=");
			if ( args != NULL ) {
				put(params, "action", args+7);
			}

			args = strstr(path, "name=");
			if ( args != NULL ) {
				put(params, "name", args + 5);
			}

			args = strstr(path, "score=");
			if ( args != NULL ) {
				put(params, "score", args + 6);
			}

			cleaner = path;
			while ( *cleaner ){
				if ( *cleaner == '&'){
					*cleaner = '\0';
				}
				++cleaner;
			}

			snake_callback(socket, params);

			free_hashtable(params);
		} else {
			output_path(socket, &path[1]);
		}

	} else if ( !strcmp(command, "POST") ){

	}
	
}

void * http_callback(void * http_data_ptr)
{

	http_data_t * http_data = (http_data_t *) http_data_ptr;

	int n, socket = (int) *http_data->socket;
	char *client_ip = http_data->client_ip, *time = http_data->accept_time;

	char *msg,first_line[BACKEND_MAX_ARRAY_SIZE], *buffer = malloc(BACKEND_MAX_BUFFER_SIZE); 

	memset(buffer, '\0', BACKEND_MAX_BUFFER_SIZE);
	n = read(socket,buffer,BACKEND_MAX_BUFFER_SIZE);

	// Getting the first line of the input

	char * c = buffer;
	int found = 0;
	unsigned long count = 0;
	while (*c && !found){
		if ( *c == '\n'){
			found = 1;
		}
		++c; ++count;
	}
	memset(first_line, '\0', sizeof(first_line));
	strncpy(first_line, buffer, count);
	first_line[sizeof(first_line) - 1] = '\0';

	c = first_line;
	while (*c) {
		if ( *c == '\n'){
			*c = '\0';
			break;
		}
		++c;
	}

	log("[%s] %s: %s\n", time, client_ip, first_line);

	// look at the first line of 'buffer' and do what you got to do..
	interpret_and_output(socket, first_line);
   
   	close(socket);
   	free(buffer);

   	free_http_data(&http_data);

   	return NULL;
}

void print_answer(const char * buffer, int socket) 
{
	char * msg;
	msg = "HTTP/1.0 200 OK\r\n";
   	write(socket,msg,strlen(msg));
   	msg = "Cache-Control: no-cache, no-store, must-revalidate\r\n";
   	write(socket,msg, strlen(msg));
   	msg = "Pragma: no-cache\r\n";
	write(socket,msg, strlen(msg));
	msg = "Expires: 0\r\n";
	write(socket,msg, strlen(msg));
   	msg = "Content-Type: text/html; charset=utf-8\r\n\r\n";
   	write(socket,msg, strlen(msg));

   	FILE * fp = fopen("games/avoid.html", "r");
   	int c;
   	while ( (c = fgetc(fp)) != EOF ){
   		write(socket, &c, 1);
   	}
   	fclose(fp);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno, n;
	socklen_t clilen; 
	time_t raw_time;
	struct tm * time_info;
	char client_IP[INET_ADDRSTRLEN];
	pthread_t input_reader_thread;
	struct sockaddr_in serv_addr, cli_addr;

	if (argc < 2) {
	 fprintf(stderr,"ERROR, no port provided\n");
	 exit(1);
	}

	scores_init();

	run_this_server_please_mister = true;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");

	clilen = sizeof(cli_addr);

	pthread_create(&input_reader_thread, NULL, input_reader_callback, NULL);

	log("Backend v.%s, c. %s %s\n",str(VERSION),__DATE__,__TIME__);
	while ( run_this_server_please_mister ){
		pthread_t callback_thread;
		listen(sockfd,5); int newsockfd_stack;
		newsockfd_stack = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		// Getting time info
		time ( &raw_time);
		time_info = localtime( &raw_time );
		char * time = asctime(time_info);
		char * c_ptr = time;
		/* the time contains a '\n'.. */
		while (*c_ptr){
			if ( *c_ptr == '\n')
				*c_ptr = '\0';
			c_ptr++;
		}

		// get client IP
		inet_ntop(AF_INET, &cli_addr.sin_addr ,client_IP, INET_ADDRSTRLEN);

		// heap allocated time
		char * time_heap = calloc(strlen(time)+1, 1);
		strcpy(time_heap, time);
		// heap allocated client ip
		char * client_ip_heap = calloc(strlen(client_IP)+1, 1);
		strcpy(client_ip_heap, client_IP);
		// sending socket info to newsockfd;
		int * newsockfd = malloc(sizeof(int));
		*newsockfd = newsockfd_stack;

		http_data_t * http_data = calloc(1, sizeof(http_data_t));
		http_data->client_ip = client_ip_heap;
		http_data->accept_time = time_heap;
		http_data->socket = newsockfd;

		pthread_create(&callback_thread, NULL, http_callback, (void*)http_data);
	}

	scores_quit();
	close(sockfd);
	return 0; 
}