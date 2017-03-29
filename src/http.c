#include "http.h"

static hashtable_t * headers_callback;
static pthread_mutex_t pthread_sync;
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
	write(socket,msg, strlen(msg));
   	msg = "Content-Type: application/javascript; charset=utf-8\r\n\r\n";
   	write(socket,msg, strlen(msg));
}

void output_txt_headers(int socket)
{
	char * msg = "HTTP/1.1 200 OK\r\n";
   	write(socket,msg,strlen(msg));
   	msg = "Cache-Control: no-cache, no-store, must-revalidate\r\n";
   	write(socket,msg, strlen(msg));
   	msg = "Pragma: no-cache\r\n";
	write(socket,msg, strlen(msg));
	msg = "Expires: 0\r\n";
	write(socket,msg, strlen(msg));
   	msg = "Content-Type: text/plain; charset=utf-8\r\n\r\n";
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
   	msg = "Cache-Control: no-cache, no-store, must-revalidate\r\n";
   	write(socket,msg, strlen(msg));
   	msg = "Pragma: no-cache\r\n";
	write(socket,msg, strlen(msg));
	msg = "Expires: 0\r\n";
	write(socket,msg, strlen(msg));
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
	char *msg, *data; 
	long int sz;
	const char * file_type;
	FILE * fp;
	int fd;
	struct stat st;

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

	fd = open(path, O_RDONLY);
	if ( fd < 0 ){
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
		close(fd);
   		return;
	}

	file_type++;

	callback = (http_header_callback_t) get(headers_callback, file_type);

	stat(path, &st);
	sz = st.st_size;

	data = malloc(sz+1);
	if ( data == NULL ){
		output_file_not_found(socket);
		close(fd);
		return;
	}

	data[sz] = '\0';

	read( fd, data, sz );

	if ( callback != NULL ){
		callback(socket);
   		write(socket, data, sz);
   		close(fd);
	} else {
		// Outputting a simple html page 
		((http_header_callback_t) get(headers_callback, "html"))(socket);

		msg = "<!DOCTYPE html><body><pre>";
		write(socket,msg, strlen(msg));
		write(socket, data, sz);
		close(fd);
		msg = "</pre></body></html>";
		write(socket,msg, strlen(msg));
	}
	free(data);
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
			if ( buffer[count] == '\n' ||buffer[count] == '\r' ){
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
* Parsing the header part of the http post data
*
* params - hash containg all keys relevant for the request
* buffer - data to the http datagram
*/
void parse_http_headers(hashtable_t * params, char * buffer){
	char *args, *c;

	args = c = buffer;

	while ( strstr(args, ": ") != NULL ){
		args = strstr(args, ": ");
		c = args;
		while ( *c != '\n' && *c!='\r' && *c && c != buffer){
			c--;
		}
		char * end_ptr = args;
		while ( *end_ptr && *end_ptr != '\n' && *end_ptr != '\r'){
			end_ptr++;
		}
		char temp = *end_ptr;
		*end_ptr = '\0';
		char * new_arg = strdup(args+2);
		*end_ptr = temp;
		end_ptr = c+1;
		while ( *end_ptr && *end_ptr != ':'){
			end_ptr++;
		}
		temp = *end_ptr;
		*end_ptr = '\0';
		char * new_key = strdup(c+1);
		*end_ptr = temp;

		put(params, new_key, new_arg);
		args += 2;

	}
}

/*
* Parsing the body part of the http post data
*
* params - hash containg all keys relevant for the request
* buffer - data to the http datagram
*/
void parse_http_post_data(hashtable_t * params, char * buffer){
	char * data = strstr(buffer, "\r\n\r\n") + 4, *delim = "&", *token,*tmp;
	
 	token = strtok(data, delim);

 	while ( token != NULL ){
 		tmp = strstr(token,"=");
 		if ( tmp != NULL ){
 			*tmp = '\0';
 			put(params, strdup(token), strdup(tmp+1));
 			*tmp = '=';
 		}

 		token = strtok(NULL, delim);
 	}
}

/*
* Parsing the body part of the http post data
*
* params - hash containg all keys relevant for the request
* buffer - data to the http datagram
*/
void parse_http_get_headers_and_arguments(hashtable_t * params, char * buffer)
{

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
		*cleaner, *buffer, *tmp;
	int cc,n;
	pid_t pid;
	hashtable_t * params, *header_params;
	FILE *fp;

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
		// Will only be looking at the first line, restfully.. 
		// Reading the last part of the request so that the connection is not reset..

		pthread_mutex_lock(&pthread_sync);
		header_params = new_hashtable(BACKEND_MAX_NBR_OF_ARGS, 0.8);
		header_params->data_also = 1;

		// Preparing the params to be casted to the post handler

		/*
		* To be continued
		*/ 

		put(header_params, strdup("path"), strdup(path));

		buffer = calloc(1024*5, 1);

		pthread_mutex_unlock(&pthread_sync);	

		n = read(socket, buffer, 1024*3);

		parse_http_headers(header_params, buffer);

		if ( !strcmp(path, "/") ){
 			// Outputting the index file!
 			pid = fork();
			if ( pid == 0 ){
				output_index(socket);
			}

   			return;
		}

		if ( strstr(path, "cgi/") != NULL && strstr(path, ".py") != NULL ){

			tmp = calloc(strlen(path)+1,1);
			strcpy(tmp, path);
			cleaner = strstr(tmp, "?");
			if ( cleaner != NULL ){
				*cleaner = '\0';
			}

			fp = fopen(tmp+1, "r");
			if ( fp != NULL ){
				fclose(fp);

				pid = fork();
				if ( pid == 0 ){
					cgi_py(socket, header_params, path); // Leaving it to the cgi writer to make sense!
				}

			} else {
				output_file_not_found(socket);
			}

			free(tmp);
		} else if ( strstr(path, "cgi/") != NULL && strstr(path, ".sh") != NULL ){

			tmp = calloc(strlen(path)+1,1);
			strcpy(tmp, path);
			cleaner = strstr(tmp, "?");
			if ( cleaner != NULL ){
				*cleaner = '\0';
			}

			fp = fopen(tmp+1, "r");
			if ( fp != NULL ){
				fclose(fp);

				pid = fork();
				if ( pid == 0 ){
					cgi_sh(socket, header_params, path); // Leaving it to the cgi writer to make sense!
				}

			} else {
				output_file_not_found(socket);
			}

			free(tmp);
		} else if ( strstr(path, "coffee.cgi") != NULL ){
			/*
			* the coffe.cgi. Args: action
			*/ 

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

			free_hashtable(params);
		} else if ( strstr(path, "game.cgi") != NULL ){
			/*
			* the game.cgi, args: action=[post_score|get_highscore], name=[*], score=[*]
			*/ 

			params = new_hashtable(BACKEND_MAX_NBR_OF_ARGS, 0.8);

			args = strstr(path, "action=");
			if ( args != NULL ) {
				put(params, "action", args+7);
			}

			snake_callback(socket, params);

			free_hashtable(params);
		} else if ( strstr(path, "index.cgi") != NULL ) {

			char *msg, *c;
			FILE *fp;

			if ( strstr(path, "action=set") != NULL ){
				msg = strstr(path, "message=");

				if ( msg == NULL ){
					output_path(socket, path+1);
					return;
				}

				fp = fopen("etc/index.txt", "w");

				c = msg;
				
				while ( *c ) {
					if ( *c == '@' ) 
						*c = ' ';
					++c;
				}
				
				fprintf(fp, "%s\n", msg+8);

				fclose(fp);

				output_path(socket, path+1);

			} else if ( strstr(path, "action=get") != NULL ){

				output_txt_headers(socket);

				fp = fopen("etc/index.txt", "r");

				if ( fp == NULL ){
					write(socket,"Lilla boppen, lilla lilla boppen", strlen("Lilla boppen, lilla lilla boppen"));
				} else {
					while( (cc=fgetc(fp)) != EOF ){
						write(socket, &cc, 1);
					}
					fclose(fp);
				}

			}

		} else {
			
			pid = fork();
			if ( pid == 0 ){
				output_path(socket, path+1);
			}
		}

		free(buffer);
		free_hashtable(header_params);

	} else if ( !strcmp(command, "POST") ){
		// This is interesting. Now i will read all arguments and parameters
		pthread_mutex_lock(&pthread_sync);

		params = new_hashtable(BACKEND_MAX_NBR_OF_ARGS, 0.8);
		params->data_also = 1;

		// Preparing the params to be casted to the post handler

		/*
		* To be continued
		*/ 

		put(params, strdup("path"), strdup(path));

		buffer = calloc(1024*5, 1);
		n = read(socket, buffer, 1024*3);

		parse_http_headers(params, buffer);
		parse_http_post_data(params, buffer);

//		print_table_as_chars(params);

		if ( strstr(path, "game.cgi") != NULL ){
			
			printf("Cookie: %s\n",(char*) get(params,"Cookie"));
			snake_callback(socket, params);

		}

//		print_table_as_chars(params);

		output_txt_headers(socket);
		write(socket, "Yes", 3);
		free(buffer);
		free_hashtable(params);

		pthread_mutex_unlock(&pthread_sync);
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
	free_hashtable(headers_callback);
}

void * http_callback(void * http_data_ptr)
{

	http_data_t * http_data = (http_data_t *) http_data_ptr;
	int socket = (int) *http_data->socket, found = 0;
	unsigned long count = 0;
	char *client_ip = http_data->client_ip, *time = http_data->accept_time;

	char first_line[BACKEND_MAX_BUFFER_SIZE], buffer[BACKEND_MAX_BUFFER_SIZE]; 

	memset(first_line, '\0', BACKEND_MAX_BUFFER_SIZE); memset(buffer, '\0', BACKEND_MAX_BUFFER_SIZE);

	while ( count < BACKEND_MAX_BUFFER_SIZE && !found ) {
		read(socket, &buffer[count], 1);
		if ( buffer[count] == '\n' ||buffer[count] == '\r' ){
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
   		free_http_data(&http_data);
		return NULL;
	}

	log("[%s] %s: %s\n", time, client_ip, first_line);

	// look at the first line of 'buffer' and do what you got to do..
	interpret_and_output(socket, first_line);
   
   	close(socket);

   	free_http_data(&http_data);

   	return NULL;
}
