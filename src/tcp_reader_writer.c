#include "tcp_reader_writer.h"

void * tcp_reader_writer_cb(void * data)
{
	char command;
	tcp_receive_operation_t * op = (tcp_receive_operation_t *) data;
	printf("Accepted connection on socket: %d from ip: %s\n", op->socket, op->client_ip);

	// Getting the command
	read(op->socket, &command, 1);
	op->command = command;

	switch ( op->command ) {
	case SEND_FILE:
		receive_file(op->socket, "downloads"); // socket closed by this function
	break;
	case SEC_SESSION:
		sec_session_server(op->socket);
	default:
	// more to come..
	break;
	}

	free(op);
	return NULL;
}

extern volatile bool run_this_server_please_mister;

void * file_receiver_thread_callback(void * data)
{
	printf("%s\n",__func__);

	int sockfd, portno = FILE_RECEIVE_PORT, newsockfd_stack;
	socklen_t clilen; 

	char client_IP[INET_ADDRSTRLEN];
	struct sockaddr_in serv_addr, cli_addr;
    struct timeval timeout;     

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){ 
		log_error("%s ERROR opening file receive socket\n", __func__);
		return NULL;
	}

    timeout.tv_sec = 120;
    timeout.tv_usec = 0;

    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0){
    	log_error("%s.%d setsockopt failed\n", __func__, __LINE__);
		return NULL;
    }

    if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0){
    	log_error("%s.%d setsockopt failed\n", __func__, __LINE__);
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

		// heap allocated request data
		tcp_receive_operation_t * op = calloc(sizeof(tcp_receive_operation_t),1);
		op->socket = newsockfd_stack;
		strcpy(op->client_ip, client_IP);

		if ( strstr(op->client_ip, "0.0.0.0") != NULL ) {
			// Result of a timeout
			continue;
		}
		
		pthread_create(&callback_thread, NULL, tcp_reader_writer_cb, op);

	}

	close(sockfd);
	return EXIT_SUCCESS; 


}