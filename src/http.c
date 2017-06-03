#include "http.h"

static hashtable_t * headers_callback;
static pthread_mutex_t pthread_sync;

/*
* A bunch of functions that output meta data
*/ 
// ----------------------------------------- //
int output_js_headers(int socket)
{
	size_t ctrl;
	char *msg;
	size_t sz;

	msg = "HTTP/1.1 200 OK\r\n\
Cache-Control: no-cache, no-store, must-revalidate\r\n\
Pragma: no-cache\r\n\
Expires: 0\r\n\
Content-Type: application/javascript; charset=utf-8\r\n\r\n";
	sz = strlen(msg);

   	ctrl = write(socket,msg, sz);
   	
   	return  ctrl != sz;
}

int output_authenticate_headers(int socket)
{

	size_t ctrl;
	char *msg;
	size_t sz;

	msg = "HTTP/1.1 401 Unauthorized\r\n\
Cache-Control: no-cache, no-store, must-revalidate\r\n\
Pragma: no-cache\r\n\
Expires: 0\r\n\
WWW-Authenticate: Basic realm=\"private\"\r\n\
Content-Type: text/html; charset=utf-8\r\n\r\n\
<!DOCTYPE html><body><pre>Authorization required.</pre></body></html>";
	sz = strlen(msg);

   	ctrl = write(socket,msg, sz);
   	
   	return  ctrl != sz;

}

void output_file_transfer_headers(int socket, char *file)
{
	/*
	* Important: the file udner path "file" must be a regular file and exist!
	*/
	unsigned sz;
	struct stat st;
	char http_line[200], *msg, *ch, *slash;
	
	msg = "HTTP/1.1 200 OK\r\n";
   	write(socket,msg,strlen(msg));
   	msg = "Cache-Control: no-cache, no-store, must-revalidate\r\n";
   	write(socket,msg, strlen(msg));
   	msg = "Pragma: no-cache\r\n";
	write(socket,msg, strlen(msg));
	msg = "Expires: 0\r\n";
	write(socket,msg, strlen(msg));
   	msg = "Content-Type: application/octet-stream\r\n";
   	write(socket,msg, strlen(msg));

   	stat(file, &st);
   	sz = st.st_size;

	memset(http_line, '\0', sizeof http_line);
	snprintf(http_line, sizeof(http_line) - 1, "Content-Length: %u\r\n", sz);

   	write(socket,http_line, strlen(http_line));

	memset(http_line, '\0', sizeof http_line);

	ch = file;
	while ( *ch ) {
		if ( *ch == '/' )
			slash = ch;
		++ch;
	}

	file = slash+1;
	*slash = '\0';

	snprintf(http_line, sizeof(http_line) - 1, "Content-Disposition: attachment; filename=\"%s\"\r\n\r\n", file);
   	write(socket,http_line, strlen(http_line));
}

int output_txt_headers(int socket)
{
	size_t ctrl;
	char *msg;
	size_t sz;

	msg = "HTTP/1.1 200 OK\r\n\
Cache-Control: no-cache, no-store, must-revalidate\r\n\
Pragma: no-cache\r\n\
Expires: 0\r\n\
Content-Type: text/plain; charset=utf-8\r\n\r\n";
	sz = strlen(msg);

   	ctrl = write(socket,msg, sz);
   	
   	return  ctrl != sz;
}

int output_json_headers(int socket)
{
	size_t ctrl;
	char *msg;
	size_t sz;

	msg = "HTTP/1.1 200 OK\r\n\
Cache-Control: no-cache, no-store, must-revalidate\r\n\
Pragma: no-cache\r\n\
Expires: 0\r\n\
Content-Type: application/json; charset=utf-8\r\n\r\n";
	sz = strlen(msg);

   	ctrl = write(socket,msg, sz);
   	
   	return  ctrl != sz;
}

int output_jpg_headers(int socket)
{
	ssize_t ctrl;
	char *msg;
	size_t sz;

	msg = "HTTP/1.1 200 OK\r\n\
Content-Description: File Transfer\r\n\
Content-Transfer-Encoding: binary\r\n\
Content-Disposition: attachment; filename=\"image.jpg\"\r\n\
Content-Type: image/jpeg\r\n\r\n";
	sz = strlen(msg);

   	ctrl = write(socket,msg, sz);
   	
   	return  ctrl != sz;
}

int output_png_headers(int socket)
{
	ssize_t ctrl;
	char *msg;
	size_t sz;

	msg = "HTTP/1.1 200 OK\r\n\
Content-Description: File Transfer\r\n\
Content-Transfer-Encoding: binary\r\n\
Content-Disposition: attachment; filename=\"image.jpg\"\r\n\
Content-Type: image/png\r\n\r\n";
	sz = strlen(msg);

   	ctrl = write(socket,msg, sz);
   	
   	return  ctrl != sz;
}

int output_css_headers(int socket)
{
	ssize_t ctrl;
	char *msg;
	size_t sz;

	msg = "HTTP/1.1 200 OK\r\n\
Cache-Control: no-cache, no-store, must-revalidate\r\n\
Pragma: no-cache\r\n\
Expires: 0\r\n\
Content-Type: text/css; charset=utf-8\r\n\r\n";
	sz = strlen(msg);

   	ctrl = write(socket,msg, sz);
   	
   	return  ctrl != sz;
}

int output_html_headers(int socket)
{
	ssize_t ctrl;
	char *msg;
	size_t sz;

	msg = "HTTP/1.1 200 OK\r\n\
Cache-Control: no-cache, no-store, must-revalidate\r\n\
Pragma: no-cache\r\n\
Expires: 0\r\n\
Content-Type: text/html; charset=utf-8\r\n\r\n";
	sz = strlen(msg);

   	ctrl = write(socket,msg, sz);

   	return  ctrl != sz;
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
* The file requested was not found on the server.. Outputs some fantastic ASCII art.
*/
void output_file_not_found(int socket)
{
	unsigned sz;
	ssize_t ctrl;
	size_t send_sz;
	int fd;
	char *data;
	struct stat st;

	char * msg = "HTTP/1.1 404 Not Found\r\n\
Cache-Control: no-cache, no-store, must-revalidate\r\n\
Pragma: no-cache\r\n\
Expires: 0\r\n\
Content-Type: text/html; charset=utf-8\r\n\r\n\
<!DOCTYPE html><body><pre>";
	send_sz = strlen(msg);

   	ctrl = write(socket,msg, send_sz);
   
   	if ( send_sz != ctrl ){
   		return;
   	}

   	fd = open("etc/404.txt", O_RDONLY);

	stat("etc/404.txt", &st);
	sz = st.st_size;

   	fd = open("etc/404.txt", O_RDONLY);
   	
   	if ( fd < 0 ){
		// sending a signal that it is no file with that name present.
		msg = "ERROR!!!?!!! (no std 404 text).";
   		write(socket,msg, strlen(msg));
		return;
	}

	data = malloc(sz+1);
	if ( data == NULL ){
		msg = "Memory error + Bad path.";
		send_sz = strlen(msg);
   		ctrl = write(socket,msg, send_sz);
		close(fd);

		if ( ctrl == send_sz ){
			// send the rest also..
			msg = "</pre></body></html>";
   			write(socket,msg, strlen(msg));
		}

		return;
	}

	data[sz] = '\0';

	read( fd, data, sz );
	ctrl = write(socket, data, sz);

	close(fd);
	free(data);

	if ( ctrl == sz ) {
		msg = "</pre></body></html>";
   		write(socket,msg, strlen(msg));
	}

	return;

}

int output_internal_server_error(int socket)
{
	int ctrl;
	char *msg;
	size_t sz;

	msg = "HTTP/1.1 500 Internal server error\r\n\
Cache-Control: no-cache, no-store, must-revalidate\r\n\
Pragma: no-cache\r\n\
Expires: 0\r\n\
Content-Type: text/html; charset=utf-8\r\n\r\n\
<!DOCTYPE html><body><pre>\
Internal server error.\
</pre></body></html>";
	sz = strlen(msg);

   	ctrl = write(socket,msg, sz);

   	return  ctrl != sz;
}

/*
* When a get request contain an URL that is a file in the 
* accessible file system, this function is responsible for 
* outputting the file.
* 
* params:
* 	@socket - tcp/ip api to write data to
* 	@path - URL of the get request
*/
void output_path(int socket, const char * path)
{
	http_header_callback_t callback;
	char *msg, *data; 
	long int sz;
	size_t msg_sz;
	const char * file_type;
	int fd, ctrl;
	struct stat st;

	// Mapping '/' to the file 'index.html'
 	if ( ! strncmp(path, "/", 2) ){

		output_html_headers(socket);
		stat("index.html", &st);
		sz = st.st_size;

	   	fd = open("index.html", O_RDONLY);
	   	
	   	if ( fd < 0 ){
			// sending a signal that it is no file with that name present.
			output_file_not_found(socket);
			return;
		}

		data = malloc(sz+1);
		if ( data == NULL ){
			printf("NO DATA!!!!!!\n");
			output_file_not_found(socket);
			close(fd);
			return;
		}

		data[sz] = '\0';

		read( fd, data, sz );
   		write(socket, data, sz);

		close(fd);
		free(data);
		return;
	}

	++path; // The paths should not be directed to the root '/' directory as base.. 

	// if someone asks for the icon, i placed it under etc 
	// so I need to add this little exception..
	if ( strstr(path, "favicon.ico") != NULL ){

		output_jpg_headers(socket);
		stat("etc/favicon.ico", &st);
		sz = st.st_size;

	   	fd = open("etc/favicon.ico", O_RDONLY);
	   	
	   	if ( fd < 0 ){
			// sending a signal that it is no file with that name present.
			output_file_not_found(socket);
			return;
		}

		data = malloc(sz+1);
		if ( data == NULL ){
			output_file_not_found(socket);
			close(fd);
			return;
		}

		data[sz] = '\0';

		read( fd, data, sz );
   		write(socket, data, sz);

		close(fd);
		free(data);
		return;
	}

	// hiding some "sensitive" information
	if ( (strstr(path, "/etc/") != NULL) ||( strstr(path, "/src/") != NULL )
			||(strstr(path, "/log/") != NULL) ||(strstr(path, "~") != NULL) 
				|| (strstr(path, "..") != NULL)) {
		
		output_file_not_found(socket);
		
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
		if ( callback(socket) ) {
			// Something went wrong, not all bytes were sent.
			close(fd);
			free(data);
			return;
		}
   		write(socket, data, sz);
   		close(fd);
	} else {
		// Outputting as a simple html page ( the extension of the file is not represented in the table )
		((http_header_callback_t) get(headers_callback, "html"))(socket);

		msg = "<!DOCTYPE html><body><pre>";
		msg_sz = strlen(msg);

		ctrl = write(socket,msg, msg_sz);
		if ( ctrl != msg_sz ){
			close(fd);
			free(data);
			return;
		}
		
		ctrl = write(socket, data, sz);
		if ( ctrl != sz ) {
			close(fd);
			free(data);
			return;
		}

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
	char *args, *c, *end_ptr, temp, *new_arg, *new_key;

	args = c = strstr(buffer,"\r\n") + 2;

	while ( strstr(args, ": ") != NULL ){
		args = strstr(args, ": ");
		c = args;
		while ( *c != '\n' && *c!='\r' && *c && c != buffer){
			c--;
		}
		end_ptr = args;
		while ( *end_ptr && *end_ptr != '\n' && *end_ptr != '\r'){
			end_ptr++;
		}
		temp = *end_ptr;
		*end_ptr = '\0';
		new_arg = strdup(args+2);
		*end_ptr = temp;
		end_ptr = c+1;
		while ( *end_ptr && *end_ptr != ':'){
			end_ptr++;
		}
		temp = *end_ptr;
		*end_ptr = '\0';
		new_key = strdup(c+1);
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
void interpret_and_output(int socket, char * data)
{

	char *c, *args, *command, *path, 
		*cleaner, *buffer, *tmp;
	int cc;
	hashtable_t * params, *header_params;
	FILE *fp;

	char first_line[BACKEND_MAX_BUFFER_SIZE]; 
	memset(first_line, '\0', sizeof first_line);

	c = strchr(data, '\r');
	if ( c != NULL )
		*c = '\0';

	snprintf(first_line, BACKEND_MAX_BUFFER_SIZE-1,"%s", data);

	if ( c != NULL )
		*c = '\r';

	// Getting the HTTP command and path!
	c = strchr(first_line, ' ');
	path = NULL;

	if ( c != NULL ) {
		*c = '\0';
		if ( path == NULL ) {
			path = &c[1]; 

			c = strchr(path, ' ');
			if ( c != NULL )
				*c = '\0';
		}	
	}

	command = first_line;

	buffer = data;

	if ( !strcmp(command, "GET") ){
		// We have recieved a 'GET' request!
		// Will only be looking at the first line, restfully.. 
		// Reading the last part of the request so that the connection is not reset..

		pthread_mutex_lock(&pthread_sync);
		header_params = new_hashtable(BACKEND_MAX_NBR_OF_ARGS, 0.8);
		pthread_mutex_unlock(&pthread_sync);
		header_params->data_also = 1;

		// Preparing the params to be casted to the post handler

		/*
		* To be continued
		*/ 

		put(header_params, strdup("path"), strdup(path));

		parse_http_headers(header_params, buffer);

		if ( strstr(path, "cgi/") != NULL && strstr(path, ".py") != NULL ){

			pthread_mutex_lock(&pthread_sync);
			tmp = calloc(strlen(path)+1,1);
			pthread_mutex_unlock(&pthread_sync);
			
			strcpy(tmp, path);
			cleaner = strstr(tmp, "?");

			if ( cleaner != NULL )
				*cleaner = '\0';

			fp = fopen(tmp+1, "r");
			if ( fp != NULL ){

				fclose(fp);

				pthread_mutex_lock(&pthread_sync);
				cgi_py(socket, header_params, path); // Leaving it to the cgi writer to make sense!
				pthread_mutex_unlock(&pthread_sync);	

			} else { 
				output_file_not_found(socket);
			}

			free(tmp);
		} else if ( strstr(path, "cgi/") != NULL && strstr(path, ".sh") != NULL ){

			pthread_mutex_lock(&pthread_sync);
			tmp = calloc(strlen(path)+1,1);
			pthread_mutex_unlock(&pthread_sync);

			strcpy(tmp, path);
			cleaner = strstr(tmp, "?");
			if ( cleaner != NULL )
				*cleaner = '\0';

			fp = fopen(tmp+1, "r");
			if ( fp != NULL ){

				fclose(fp);
				
				pthread_mutex_lock(&pthread_sync);
				cgi_sh(socket, header_params, path); // Leaving it to the cgi writer to make sense!
				pthread_mutex_unlock(&pthread_sync);

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
					free_hashtable(header_params);
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
					write(socket,"Hej", 3);
				} else {
					while( (cc=fgetc(fp)) != EOF ){
						write(socket, &cc, 1);
					}
					fclose(fp);
				}

			}

		} else if ( strstr(path, "list.cgi" ) ) {
			char *list_action_cgi, *list_path_cgi, auth_info[100], *auth;
			unsigned char *auth_decoded;
			size_t out_len;
			hashtable_t *display_parameters;

			memset(auth_info, '\0', sizeof auth_info);

			auth = (char*) get(header_params, "Authorization");
			if ( auth == NULL ) {
				output_authenticate_headers(socket);			
				free_hashtable(header_params);
				return;
			} 

			auth_decoded = base64_decode(auth + 6, strlen(auth) - strlen("Basic "), &out_len);

			printf("auth_decoded: %s\n", auth_decoded);
			if ( strcmp((char*)auth_decoded, STANDARD_USER_PASSWORD) ){
				// Wrong user or password or both given!
				output_authenticate_headers(socket);
				free_hashtable(header_params);
				free(auth_decoded);
				base64_cleanup(); 
				return;
			}
			
			free(auth_decoded);
			base64_cleanup(); 

			display_parameters = new_hashtable(BACKEND_MAX_NBR_OF_ARGS, 0.8);
			display_parameters->data_also = 1;

			if ( strstr(path, "path=") == NULL ||strstr(path, "action=") == NULL ||strchr(path, '&') == NULL ){
				output_file_not_found(socket);
				free_hashtable(header_params);
				return;
			} 

			list_path_cgi = strstr(path, "path=") + 5;
			list_action_cgi = strstr(path, "action=") + 7;

			*(strchr(path, '&')) = '\0'; // This is already included

			put(display_parameters, strdup("path"), strdup(list_path_cgi));
			put(display_parameters, strdup("action"), strdup(list_action_cgi));

			file_display_cgi(socket, display_parameters);

			free_hashtable(display_parameters);

		} else {
			
			output_path(socket, path);

		}

		free_hashtable(header_params);

	} else if ( !strcmp(command, "POST") ){
		// This is interesting. Now i will read all arguments and parameters
		pthread_mutex_lock(&pthread_sync);
		params = new_hashtable(BACKEND_MAX_NBR_OF_ARGS, 0.8);
		pthread_mutex_unlock(&pthread_sync);
		
		params->data_also = 1;

		// Preparing the params to be casted to the post handler

		put(params, strdup("path"), strdup(path));

		buffer = data;

		parse_http_headers(params, buffer);
		parse_http_post_data(params, buffer);

		if ( strstr(path, "game.cgi") != NULL )
			snake_callback(socket, params);

		output_txt_headers(socket);
		write(socket, "Yes", 3);
		free_hashtable(params);

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
	put(headers_callback, "png", output_png_headers);
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
	int socket = (int) *http_data->socket;
	char *client_ip = http_data->client_ip, *time = http_data->accept_time;
	size_t read_sz = 0, buffer_size = BUFFER_INCREMENT;

	char * buffer = calloc(buffer_size, 1), *first_line, *c;

	if ( buffer == NULL ){
		log_error("%s.%d Failed to allocate %zu bytes.\n", __func__, __LINE__, buffer_size);
		goto end2;
	}

	read_sz = read(socket, buffer, buffer_size);

	if ( read_sz == 0 )
		goto end1;

	while ( read_sz == buffer_size && read_sz < MAXIMUM_READ_SIZE ) {

		buffer_size += BUFFER_INCREMENT;

		buffer = realloc(buffer, buffer_size);

		memset(buffer + read_sz, '\0', BUFFER_INCREMENT);

		if ( buffer == NULL ){
			log_error("%s.%d Failed to allocate %zu bytes.\n", __func__, __LINE__, buffer_size);
			goto end2;
		}

		read_sz += read(socket, buffer + read_sz, BUFFER_INCREMENT);

	}

	c = strchr(buffer, '\r');
	if ( c != NULL ) 
		*c = '\0';

	first_line = buffer;

	log("[%s] %s: %s\n", time, client_ip, first_line);

	if ( c != NULL )
		*c = '\r';

	// look at the first line of 'buffer' and do what you got to do..
	interpret_and_output(socket, buffer);
end1:
   	free(buffer);
end2:  
   	close(socket);

   	free_http_data(&http_data);

   	return NULL;

}

