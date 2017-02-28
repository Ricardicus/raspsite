#include "tcp_file_reader.h"

void * tcp_receive_file(void * data)
{
	int socket = *((int*) data);
	printf("Accepted connection on socket: %d\n", socket);
	receive_file(socket, "downloads");
	return NULL;
}

extern volatile bool run_this_server_please_mister;

void * file_receiver_thread_callback(void * data)
{
	printf("%s\n",__func__);

	int sockfd, portno = FILE_RECEIVE_PORT, newsockfd_stack;
	socklen_t clilen; 

	char client_IP[INET_ADDRSTRLEN], command;
	struct sockaddr_in serv_addr, cli_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){ 
		log_error("%s ERROR opening file receive socket\n", __func__);
		return NULL;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = FILE_RECEIVE_PORT;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		log_error("%s ERROR on file receive binding\n", __func__);
		return NULL;
	}

	clilen = sizeof(cli_addr);

	while ( run_this_server_please_mister ){
		pthread_t callback_thread;
		listen(sockfd,5);
		newsockfd_stack = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		// get client IP
		inet_ntop(AF_INET, &cli_addr.sin_addr ,client_IP, INET_ADDRSTRLEN);

		// heap allocated client ip
		char * client_ip_heap = calloc(strlen(client_IP)+1, 1);
		strcpy(client_ip_heap, client_IP);

		log("File transfer request from IP: %s\n", client_ip_heap);
		free(client_ip_heap);

		// Getting the command
		read(newsockfd_stack, &command, 1);
		
		if ( command == SEND_FILE ){
			pthread_create(&callback_thread, NULL, tcp_receive_file, &newsockfd_stack);
		}

	}

	close(sockfd);
	return EXIT_SUCCESS; 


}