/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void print_answer(const char * buffer, int socket, int size) 
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
   	msg = "Content-Type: text/plain; charset=utf-8\r\n\r\n";
   	write(socket,msg, strlen(msg));
   	msg = "I got your message.\r\n";
   	write(socket,msg,strlen(msg));
   	msg = "It was this:\r\n";
    write(socket, msg, strlen(msg));
    write(socket, buffer, size);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	
	char * buffer = malloc(600);
	memset(buffer, '\0', 600);

	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
	 fprintf(stderr,"ERROR, no port provided\n");
	 exit(1);
	}
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

	for ( int i = 0; i < 10; ++i){
		listen(sockfd,5);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		printf("just accepted a connection, i:%d\n", i);
		if (newsockfd < 0)
		  printf("ERROR on accept\n");

		memset(buffer, '\0', 600);
		n = read(newsockfd,buffer,600);

		printf("printing:\n");
		print_answer(buffer, newsockfd, 600);

		if (n < 0){
			printf("ERROR reading from socket\n");
		}

		close(newsockfd);
		//printf("Here is the message: %s\n",buffer);
	}

	close(sockfd);

	free(buffer);
	return 0; 
}