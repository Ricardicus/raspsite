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
	int socket, response;

	if ( argc < 4 ){
		printf("usage: ./write [server-IP][ file] [received file name]\n");
		return -1;
	}

	socket = make_contact(argv[1], 8090);

	if ( socket == -1 ){
		log_error("Failed to connect to server.");
		return -1;
	}

	printf("Attempting to output file\n");
	
	response = output_file(socket, argv[2], argv[3]);
	
	switch (response) {
	case EVERYTHING_OK:
		printf("File received on host.\n");
	break;
	default:
		printf("Consult the log file, something went wrong during file send operation. Is the server responding?");
	break;
	}

	close(socket);

	return 0;
}
