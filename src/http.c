#include "http.h"

static hashtable_t * headers_callback;

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
	
	msg = "HTTP/1.1 200 OK\r\n\
Cache-Control: no-cache, no-store, must-revalidate\r\n\
Pragma: no-cache\r\n\
Expires: 0\r\n\
Content-Type: application/octet-stream\r\n";
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

int output_woff2_headers(int socket)
{
	ssize_t ctrl;
	char *msg;
	size_t sz;

	msg = "HTTP/1.1 200 OK\r\n\
Cache-Control: no-cache, no-store, must-revalidate\r\n\
Pragma: no-cache\r\n\
Expires: 0\r\n\
Content-Type: application/font-woff2; charset=utf-8\r\n\r\n";
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
* Parsing the data from a http request
*
* params - hash containg all keys relevant for the request
* buffer - data to the http datagram
*/
void parse_http_get_headers_and_arguments(hashtable_t * params, char * buffer, size_t size)
{
	char line_buffer[MAX_LINE_SIZE], *end, *start, *c, *url, *path,
			*args_start, *args_end, *version;
	int first = 1;

	memset(line_buffer, '\0', MAX_LINE_SIZE);
	start = buffer;
	end = strchr(start, '\n');

	while ( end != NULL ) {
		*end = '\0';

		snprintf(line_buffer, sizeof(line_buffer) - 1, "%s", start);

		if ( first ) {
			// Assumes: GET /resource.type?arg1=value1&arg2=value2 HTTP/1.1 
			// The command (GET/POST/HEAD) will get passed env. variable: COMMAND
			// The URL to the env. variable: URL
			// Arguments will be passed as is, but with capital letters and they will be
			// expanded to their values respectively, e.g. ARG1=value1.. 
			// HTTP/1.1 will be loaded to VERSION, e.g. VERSION=HTTP/1.1
			// * Exception:
			//		when path only is '/' as URL, it will be translated into 'index.html'.

			c = strchr(line_buffer, ' ');
			if ( c != NULL ) {
				*c = '\0';
				put(params, strdup("COMMAND"), strdup(line_buffer));

				*c = ' ';
				++c;

				url = c;
				version = strchr(c, ' ');

				if ( *c != '\0' && (c = strchr(c, ' ')) != NULL ) {

					*c = '\0';

					put(params, strdup("URL"), strdup(url));

					*c = ' ';
					++c;

					path = url;

					if ( (c = strchr(path, '?')) != NULL ) {

						*c = '\0';

						if ( !strncmp(path, "/ ", 2) ) {
							put(params, strdup("PATH"), strdup("index.html"));
						} else {
							put(params, strdup("PATH"), strdup(path+1));
						}

						// Continue with parsing the arguments
						*c = '?';
						c++; // now it points to the start of arg=value[&args=values..]

						args_start = c;
						args_end = strchr(args_start, '&');

						if ( args_end != NULL ) {

							while ( args_end != NULL ) {

								*args_end = '\0';

								c = strchr(args_start, '=');
								if ( c != NULL ) {
									*c = '\0';

									put( params, strdup(args_start), strdup(c+1) );

								}

								*args_end = '&';

								args_start = args_end+1;
								args_end = strchr(args_start, '&');

								if ( args_end == NULL ) {

									if ( (c = strchr(args_start, '=')) != NULL ) {

										*c = '\0';

										char * p = strchr(c+1, ' ');
										if ( p != NULL )
											*p = '\0';

										put(params, strdup(args_start), strdup(c+1) );

										if ( p != NULL )
											*p = ' ';

									}	

								} 

							}

						} else if ( (c = strchr(args_start, '=')) != NULL ) {

							*c = '\0';
							char * p = strchr(c+1, ' ');
							if ( p != NULL )
								*p = '\0';

							put(params, strdup(args_start), strdup(c+1) );

							if ( p != NULL )
								*p = ' ';

						}


					} else if ( (c = strchr(path, ' ')) != NULL ) {

						*c = '\0';
						if ( !strncmp(path, "/", 2) ) {
							put(params, strdup("PATH"), strdup("index.html"));
						} else {
							put(params, strdup("PATH"), strdup(path+1));
						}

					}

				}

				if ( version != NULL ) {

					++version;
					if ( ( c = strchr(version, '\r') ) != NULL )
						*c = '\0';
					
					put(params, strdup("VERSION"), strdup(version));

				}

			}

			first = 0;
		
		} else {

			if ( ( c = strchr(line_buffer, ':') ) != NULL ) {
				*c = '\0';
				char * clean;
				if ( (clean=strchr(c+2, '\r')) != NULL )
					*clean = '\0';
				if ( (clean=strchr(c+2, '\n')) != NULL )
					*clean = '\0';

				put( params, strdup(line_buffer), strdup(c+2) );
			}

		}

		end++;
		start = end;
		end = strchr(end, '\n');

		memset(line_buffer, '\0', sizeof line_buffer);
	}
}

/*
* Reads the first line message of the HTTP client request
* and decides what to do. 
*
* socket - read/write data stream descriptor in the tcp/ip API
* first_line - the first line of the GET request
*/
void interpret_and_output(int socket, char * data, size_t size)
{
	char *command, *path, *msg, *c,
		*list_action_cgi, *list_path_cgi, auth_info[100], *auth;
	unsigned char *auth_decoded;
	int cc;
	hashtable_t * params;
	FILE *fp;
	size_t out_len;

	params = new_hashtable(30, 0.8);
	params->data_also = 1;

	parse_http_get_headers_and_arguments(params, data, size);

	path = get(params, "PATH");

	if ( !strcmp( get(params, "COMMAND") , "GET") ){
		// We have recieved a 'GET' request!
		// Will only be looking at the first line, restfully.. 
		// Reading the last part of the request so that the connection is not reset..

		if ( strstr(path, "cgi/") != NULL && strstr(path, ".py") != NULL ){

			fp = fopen(path, "r");
			if ( fp != NULL ){

				fclose(fp);

				cgi_py(socket, params, path); // Leaving it to the cgi writer to make sense!	

			} else { 
				output_file_not_found(socket);
			}

		} else if ( strstr(path, "cgi/") != NULL && strstr(path, ".sh") != NULL ){

			fp = fopen(path, "r");
			if ( fp != NULL ){

				fclose(fp);
				
				cgi_sh(socket, params, path); // Leaving it to the cgi writer to make sense!

			} else {
				output_file_not_found(socket);
			}

		} else if ( strstr(path, "coffee.cgi") != NULL ){
			/*
			* the coffe.cgi. Args: action
			*/ 

			output_coffee_action(socket, get(params, "action"));

		} else if ( strstr(path, "game.cgi") != NULL ){
			/*
			* the game.cgi, args: action=[post_score|get_highscore], name=[*], score=[*]
			*/ 

			snake_callback(socket, params);

		} else if ( strstr(path, "index.cgi") != NULL ) {

			if ( !strncmp(get(params, "action"), "set",3) ){
				msg = get(params, "message");

				if ( msg == NULL ){
					output_path(socket, path);
					return;
				}

				fp = fopen("etc/index.txt", "w");

				c = msg;
				
				while ( *c ) {
					if ( *c == '@' ) 
						*c = ' ';
					++c;
				}
				
				fprintf(fp, "%s\n", msg);

				fclose(fp);

				output_path(socket, path);

			} else if ( !strcmp( get(params, "action"), "get") ){

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

		} else if ( !strcmp(path, "list.cgi" ) ) {

			memset(auth_info, '\0', sizeof auth_info);

			auth = (char*) get(params, "Authorization");
			if ( auth == NULL ) {
				output_authenticate_headers(socket);			
				return;
			} 

			auth_decoded = base64_decode(auth + 6, strlen(auth) - strlen("Basic "), &out_len);

			printf("auth_decoded: %s\n", auth_decoded);
			if ( strcmp((char*)auth_decoded, STANDARD_USER_PASSWORD) ){
				// Wrong user or password or both given!
				output_authenticate_headers(socket);
				free(auth_decoded);
				base64_cleanup(); 
				return;
			}
			
			free(auth_decoded);
			base64_cleanup(); 

			if ( get(params, "path") == NULL || get(params, "action") == NULL ){
				output_file_not_found(socket);
				return;
			} 

			list_path_cgi = get(params, "path");
			list_action_cgi = get(params, "action");

			file_display_cgi(socket, params);

		} else {
			
			output_path(socket, path);

		}

	} else if ( !strcmp(command, "POST") ){
		// This is interesting. Now i will read all arguments and parameters
		// Preparing the params to be casted to the post handler

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
	put(headers_callback, "woff2", output_woff2_headers);
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
	interpret_and_output(socket, buffer, buffer_size);
end1:
   	free(buffer);
end2:  
   	close(socket);

   	free_http_data(&http_data);

   	return NULL;

}

