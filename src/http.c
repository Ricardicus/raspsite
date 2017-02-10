#include "http.h"

static hashtable_t * headers_callback;
/*
* A bunch of functions that output meta data
*/ 
// ----------------------------------------- //
void output_js_headers(int socket)
{
	char * msg = "HTTP/1.1 200 OK\r\n";
   	write(socket,msg,strlen(msg));
   	msg = "Cache-Control: no-cache, no-store, must-revalidate\r\n";
   	write(socket,msg, strlen(msg));
   	msg = "Pragma: no-cache\r\n";
	write(socket,msg, strlen(msg));
	msg = "Expires: 0\r\n";
   	msg = "Content-Type: application/javascript; charset=utf-8\r\n\r\n";
   	write(socket,msg, strlen(msg));
}

void output_json_headers(int socket)
{
	char * msg = "HTTP/1.1 200 OK\r\n";
   	write(socket,msg,strlen(msg));
   	msg = "Cache-Control: no-cache, no-store, must-revalidate\r\n";
   	write(socket,msg, strlen(msg));
   	msg = "Pragma: no-cache\r\n";
	write(socket,msg, strlen(msg));
	msg = "Expires: 0\r\n";
   	msg = "Content-Type: application/json; charset=utf-8\r\n\r\n";
   	write(socket,msg, strlen(msg));
}

void output_jpg_headers(int socket)
{
	char * msg = "HTTP/1.1 200 OK\r\n";
   	write(socket,msg,strlen(msg));
	msg = "Content-Description: File Transfer\r\n";
   	write(socket,msg, strlen(msg));	
   	msg = "Content-Transfer-Encoding: binary\r\n";
   	write(socket,msg, strlen(msg));	
   	msg = "Content-Disposition: attachment; filename=\"image.jpg\"\r\n";
   	write(socket,msg, strlen(msg));	
	msg = "Content-Type: image/jpeg\r\n\r\n";
  	write(socket,msg, strlen(msg));
}

void output_css_headers(int socket)
{
	char * msg = "HTTP/1.1 200 OK\r\n";
   	write(socket,msg,strlen(msg));
	msg = "Content-Type: text/css; charset=utf-8\r\n\r\n";
   	write(socket,msg, strlen(msg));
}

void output_html_headers(int socket)
{
	char * msg = "HTTP/1.1 200 OK\r\n";
   	write(socket,msg,strlen(msg));
   	msg = "Cache-Control: no-cache, no-store, must-revalidate\r\n";
   	write(socket,msg, strlen(msg));
   	msg = "Pragma: no-cache\r\n";
	write(socket,msg, strlen(msg));
	msg = "Expires: 0\r\n";
	write(socket,msg, strlen(msg));
   	msg = "Content-Type: text/html; charset=utf-8\r\n\r\n";
   	write(socket,msg, strlen(msg));
}

// ----------------------------------------- //

/*
* Says goodbye to some heap allocated memory 
*/
void free_http_data(http_data_t ** http_data)
{
	free((*http_data)->client_ip);
	free((*http_data)->accept_time);
	free((*http_data)->socket);
	free((*http_data));
	*http_data = NULL;
}

/*
* Writes the index page to the tcp/ip socket API
*/
void output_index(int socket)
{
	char * msg = "HTTP/1.1 200 OK\r\n";
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

/*
* The file requested was not found on the server.. Outputs some fantastic ASCII art.
*/
void output_file_not_found(int socket)
{
	char * msg = "HTTP/1.1 404 Not Found\r\n";
   	write(socket,msg,strlen(msg));
   	msg = "Cache-Control: no-cache, no-store, must-revalidate\r\n";
   	write(socket,msg, strlen(msg));
   	msg = "Pragma: no-cache\r\n";
	write(socket,msg, strlen(msg));
	msg = "Expires: 0\r\n";
	write(socket,msg, strlen(msg));
   	msg = "Content-Type: text/html; charset=utf-8\r\n\r\n";
   	write(socket,msg, strlen(msg));

   	msg = "<!DOCTYPE html><body><pre>";
   	write(socket,msg, strlen(msg));
   	
   	FILE * fp = fopen("etc/404.txt", "r");
   	int c;
   	while ( (c = fgetc(fp)) != EOF ){
   		write(socket, &c, 1);
   	}
   	fclose(fp);

   	msg = "</pre></body></html>";
   	write(socket,msg, strlen(msg));

}

/*
* When a get request contain an URL that is a file in the 
* accessible file system, this function is responsible for 
* outputting the file.
* 
* socket - tcp/ip api to write data to
* path - URL of the get request
*/
void output_path(int socket, const char * path)
{
	http_header_callback_t callback;
	char * msg; 
	const char * file_type;
	FILE * fp;

	// hiding some "sensitive" information
	if ( (strstr(path, "/etc/") != NULL) ||( strstr(path, "/src/") != NULL )
			||(strstr(path, "/log/") != NULL) ||(strstr(path, "~") != NULL) 
				|| (strstr(path, "..") != NULL)) {
		
		output_file_not_found(socket);
		
		return;
	} 

	// if someone asks for the icon, i placed it under etc 
	// so I need to add this little exception..
	if ( strstr(path, "favicon.ico") != NULL ){

		output_jpg_headers(socket);
	   	fp = fopen("etc/favicon.ico", "r");
	   	int c;
	   	while ( (c = fgetc(fp)) != EOF ){
	   		write(socket, &c, 1);
	   	}
	   	fclose(fp);
		return;

	}

	/*
	* Outputs file if it exists
	*/

	fp = fopen(path, "r");
	if ( ! fp ){
		// sending a signal that it is no file with that name present.
		output_file_not_found(socket);
		return;
	}
	
	file_type = path;
	
	while (*file_type && *file_type != '.'){
		file_type++;
	}

	if ( !*file_type ){
		// if the file type_was not found.. 
		log("error: file type of requested resource %s was not found.\n", path);
		
		output_file_not_found(socket);
		fclose(fp);
   		return;
	}

	file_type++;

	callback = (http_header_callback_t) get(headers_callback, file_type);

	if ( callback != NULL ){
		callback(socket);
		int c;
		while ( (c = fgetc(fp)) != EOF ){
   			write(socket, &c, 1);
   		}
   		fclose(fp);
   		return;
	} else {
		// Outputting a simple html page 
		((http_header_callback_t) get(headers_callback, "html"))(socket);

		msg = "<!DOCTYPE html><body><pre>";
		write(socket,msg, strlen(msg));
		int c;
		while ( (c = fgetc(fp)) != EOF ){
			write(socket, &c, 1);
		}
		fclose(fp);
		msg = "</pre></body></html>";
		write(socket,msg, strlen(msg));

	}
}

/*
* Reading HTTP data line by line, returns 1 if it sees the 
* marker '\r\n\r\n' that means that the header body has ended
*
*	buffer - pointer to buffer_size allocated bytes of data
*	socket - the tcp/ip api to read data from
*/
int get_next_line(char * buffer, int buffer_size, int socket)
{
		int count = 0, found = 0, r = 1;
		memset(buffer, '\0', buffer_size); 

		while ( count < buffer_size && !found && r > 0) {
			r = read(socket, &buffer[count], 1);
			if ( buffer[count] == '\n' || buffer[count] == '\r' ){
				// end of first line (in time)
				found = 1;
				buffer[count] = '\0';
			} 
			++count;
		}

		if ( !found ){
			// cutting this argument.. It was to large ( probably because of cookies.. )
			buffer[--count] = '\0';
		}

		if ( strstr(buffer, "\r\n\r\n") != NULL ){
			return 0;
		}

		return 1;

}

/*
* Reads the first line message of the HTTP client request
* and decides what to do. 
*
* socket - read/write data stream descriptor in the tcp/ip API
* first_line - the first line of the GET request
*/
void interpret_and_output(int socket, char * first_line)
{

	char *c, *args, *command, *path, 
		*cleaner, *buffer;
	int arg_count, count, cc;
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

		if ( strstr(path, "coffee.cgi") != NULL ){
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

			output_coffee_action(socket, args + 7);

			free_hashtable(params,0);
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

			free_hashtable(params,0);
		} else {
			output_path(socket, path);
		}

	} else if ( strstr(command, "index.cgi") != NULL ) {

		char *action, *msg, *c;
		FILE *fp;

		if ( strstr(path, "action=set") != NULL ){
			msg = strstr(path, "message=");

			if ( msg == NULL ){
				output_path(socket, path);
				return;
			}

			fp == fopen("etc/index.txt", "w");

			c = msg + 8;
			while ( *c ) {
				if ( c == '@' ) 
					*c = ' ';
				++c;
			}

			fprintf(fp, "%s", msg);
			fclose(fp);
		} else if ( strstr(path, "action=set") != NULL ){
			msg = strstr(path, "message=");

			if ( msg == NULL ){
				output_path(socket, path);
				return;
			}

			fp == fopen("etc/index.txt", "w");

			c = msg + 8;
			while ( *c ) {
				if ( *c == '@' ) 
					*c = ' ';
				++c;
			}

			fprintf(fp, "%s", msg);
			fclose(fp);
		} else if ( strstr(path, "action=get") != NULL ){

			fp == fopen("etc/index.txt", "r");

			if ( fp == NULL ){
				write(socket,"Lilla boppen, lilla lilla boppen", strlen("Lilla boppen, lilla lilla boppen"));
			} else {
				while( (cc=fgetc(fp)) != EOF ){
					write(socket, &cc, 1);
				}
			}

			fclose(fp);

		} 

	} else if ( !strcmp(command, "POST") ){
		// This is interesting. Now i will read all arguments and parameters

		// To be continued.. 
		params = new_hashtable(BACKEND_MAX_NBR_OF_ARGS, 0.8);

		put(params, "path", strdup(path));

		buffer = calloc(BACKEND_MAX_BUFFER_SIZE, 1);


		free(buffer);
		free_hashtable(params,1);
	}
	
}

void http_init()
{
	headers_callback = new_hashtable(15, 0.75);
	put(headers_callback, "js", output_js_headers);
	put(headers_callback, "css", output_css_headers);
	put(headers_callback, "json", output_json_headers);
	put(headers_callback, "html", output_html_headers);
	// Map all kinds of things to jpg for this server..
	put(headers_callback, "jpg", output_jpg_headers);
	put(headers_callback, "jpeg", output_jpg_headers);
	put(headers_callback, "gif", output_jpg_headers);
	put(headers_callback, "ico", output_jpg_headers);
}

void http_quit()
{
	free_hashtable(headers_callback,0);
}

void * http_callback(void * http_data_ptr)
{

	http_data_t * http_data = (http_data_t *) http_data_ptr;
	int n, socket = (int) *http_data->socket;
	unsigned long count = 0;
	char *client_ip = http_data->client_ip, *time = http_data->accept_time, *c;

	char first_line[BACKEND_MAX_ARRAY_SIZE], buffer[BACKEND_MAX_ARRAY_SIZE]; 

	memset(first_line, '\0', BACKEND_MAX_BUFFER_SIZE); memset(buffer, '\0', BACKEND_MAX_BUFFER_SIZE);

	while ( count < BACKEND_MAX_BUFFER_SIZE && !found ) {
		read(socket, &buffer[count], 1);
		if ( buffer[count] == '\n' || buffer[count] == '\r' ){
			// end of first line (in time)
			found = 1;
		} else {
			first_line[count] = buffer[count];
		}
		++count;
	}

	// Getting the first line of the input
	if ( found == 0 ){
		// disregard this.
		log("[%s] %s: %s [DISREGARDED]\n", time, client_ip, first_line);
		return;
	}

	log("[%s] %s: %s\n", time, client_ip, first_line);

	// look at the first line of 'buffer' and do what you got to do..
	interpret_and_output(socket, first_line);
   
   	close(socket);
   	free(buffer);

   	free_http_data(&http_data);

   	return NULL;
}