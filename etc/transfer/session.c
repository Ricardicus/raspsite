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
#include "sec_session.h"

#define THE_FILE_TO_SEND	"hej.txt"
#define FILE_SEND_PORT		8090

int main(int argc, char *argv[])
{
	int socket, response;

	if ( argc < 2){
		printf("usage: ./write [server-IP]\n");
		return -1;
	}

	socket = make_contact(argv[1], 8090);

	if ( socket == -1 ){
		log_error("Failed to connect to server.");
		return -1;
	}

	sec_session_client(socket);

	close(socket);

	return 0;
}
