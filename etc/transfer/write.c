#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include "notif.h" 

#define THE_FILE_TO_SEND	"hej.txt"
#define FILE_SEND_PORT		8090

int main(int argc, char *argv[])
{
/*	
	for ( int i = 1; i<=100000; i++){ // Stupid DDos attempt

		printf("\n--- Connection: %d\n\n", i);

		int socket = make_contact("194.47.245.253", 80);
		char buffer[2000];
		memset(buffer,'\0',sizeof(buffer));
		printf("socket: %d\n", socket);
		char * request = "GET / HTTP/1.1\r\nHost: etf.nu\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36\r\nAccept: text/html,application/xhtml+xml\r\nAccept-Language: sv-SE,sv;q=0.8,en-US;q=0.6,en;q=0.4\r\n\r\n";
		write(socket, request, strlen(request));
		read(socket, buffer, 2000);
		printf("buffer:%s\n", buffer);
		//output_file(socket, "hej.txt", "hejsan.txt");
		close(socket);

	} 

*/

	if ( argc < 4 ){
		printf("usage: ./write [server-IP][ file] [received file name]\n");
		return -1;
	}

	int socket = make_contact(argv[1], 8090);

	if ( socket == -1 ){
		log_error("Failed to connect to server.");
		return -1;
	}

	printf("Attempting to output file\n");
	output_file(socket, argv[2], argv[3]);
	printf("File received on host.\n");
	close(socket);

	return 0;
}
