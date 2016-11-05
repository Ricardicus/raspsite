#include "coffee.h"

void write_current_coffe(int socket)
{

	char date[10];
	memset(date, '\0', sizeof(date));

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	sprintf(date, "%d%d%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	int i = 0;

	char * c = date;
	while (*c){
		i += *c;
		++c;
	}

	char * msg;

	switch( i % NUMBER_OF_COFFEES) {
	case ZOEGA:
		msg = "zoega";
   		write(socket,msg, strlen(msg));
		break;
	case CAFFE_MACCHIATO:
		msg = "macchiato";
   		write(socket,msg, strlen(msg));
		break;
	case ESPRESSO:
		msg = "espresso";
   		write(socket,msg, strlen(msg));
		break;
	case CAPPUCCINO:
		msg = "cappuccino";
   		write(socket,msg, strlen(msg));
		break;
	}

	close(socket);
}

void output_coffee_action(int socket, char * action)
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