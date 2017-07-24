/* Server in the internet domain using TCP
   Port passed as an argument */
#include "backend.h"

volatile bool run_this_server_please_mister;

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

int main(int argc, char *argv[])
{
	int sockfd, portno, newsockfd_stack;
	socklen_t clilen; 
	time_t raw_time;
	struct tm * time_info;
	char client_IP[INET_ADDRSTRLEN];
	pthread_t input_reader_thread, file_reader_thread;
	struct sockaddr_in serv_addr, cli_addr;
	char * time_heap, * client_ip_heap, *time_c, *c_ptr;
	int * newsockfd;

	if (argc < 2) {
		log_error("ERROR, no port provided\n");
		return EXIT_FAILURE;
	}

	scores_init();
	http_init();

	run_this_server_please_mister = true;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){ 
		log_error("ERROR opening socket");
		scores_quit();
		return EXIT_FAILURE;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		log_error("ERROR on binding\n");
		scores_quit();
		return EXIT_FAILURE;
	}

	clilen = sizeof(cli_addr);

	pthread_create(&input_reader_thread, NULL, input_reader_callback, NULL);
	pthread_create(&file_reader_thread, NULL, file_receiver_thread_callback, NULL);

	log("Backend v.%s, c. %s %s\n",str(VERSION),__DATE__,__TIME__);
	while ( run_this_server_please_mister ){
		pthread_t callback_thread;
		listen(sockfd,20);
		newsockfd_stack = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		// Getting time info
		time ( &raw_time);
		time_info = localtime( &raw_time );
		time_c = asctime(time_info);
		c_ptr = strchr(time_c, '\n');
		/* the time contains a '\n'.. */
		if (c_ptr != NULL )
			*c_ptr = '\0';

		// get client IP
		inet_ntop(AF_INET, &cli_addr.sin_addr ,client_IP, INET_ADDRSTRLEN);

		// heap allocated time
		time_heap = calloc(strlen(time_c)+1, 1);
		
		if (time_heap == NULL) {
			log_error("%s.%d calloc failed",__func__, __LINE__);
			close(newsockfd_stack);
			continue;
		}

		strcpy(time_heap, time_c);
		// heap allocated client ip
		client_ip_heap = calloc(strlen(client_IP)+1, 1);

		if ( client_ip_heap == NULL ) {
			log_error("%s.%d calloc failed", __func__, __LINE__);
			free(time_heap);
			close(newsockfd_stack);
			continue;
		}

		strcpy(client_ip_heap, client_IP);
		// sending socket info to newsockfd;
		newsockfd = malloc(sizeof(int));

		if ( newsockfd == NULL ) {
			log_error("%s.%d malloc failed", __func__, __LINE__);
			free(time_heap);
			free(client_ip_heap);
			close(newsockfd_stack);			
			continue;
		}

		*newsockfd = newsockfd_stack;

		http_data_t * http_data = calloc(1, sizeof(http_data_t));

		if ( http_data == NULL ) {
			log_error("%s.%d calloc failed", __func__, __LINE__);
			free(time_heap);
			free(client_ip_heap);
			free(newsockfd);
			close(newsockfd_stack);
			continue;
		}

		http_data->client_ip = client_ip_heap;
		http_data->accept_time = time_heap;
		http_data->socket = newsockfd;

		if ( pthread_create(&callback_thread, NULL, http_callback, (void*)http_data) ) {
			log_error("%s.%d pthread_create failed", __func__, __LINE__);
			free_http_data(&http_data);
			close(newsockfd);
			continue;
		}

		pthread_detach(callback_thread);

	}

	scores_quit();
	http_quit();
	close(sockfd);

	return EXIT_SUCCESS; 
}
