#include "http.h"

static hashtable_t * headers_callback;

void output_js_headers(int socket)
{
	char * msg = "HTTP/1.1 200 OK\r\n";
   	write(socket,msg,strlen(msg));
   	msg = "Content-Type: application/javascript; charset=utf-8\r\n\r\n";
   	write(socket,msg, strlen(msg));
}

void output_json_headers(int socket)
{
	char * msg = "HTTP/1.1 200 OK\r\n";
   	write(socket,msg,strlen(msg));
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

void free_http_data(http_data_t ** http_data)
{
	free((*http_data)->client_ip);
	free((*http_data)->accept_time);
	free((*http_data)->socket);
	free((*http_data));
	*http_data = NULL;
}

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

void output_path(int socket, const char * path)
{
	http_header_callback_t callback;
	char * msg; 
	FILE * fp;

	/*
	* Exceptions to the file output
	*/  

	// hiding some sensisitve information
	if ( (strstr(path, "/etc/") != NULL) ||( strstr(path, "/src/") != NULL )||(strstr(path, "/log/") != NULL)){
		output_file_not_found(socket);
		return;
	} 

	// if google chrome asks for icon
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
	* Output file if it exists
	*/

	fp = fopen(path, "r");
	if ( ! fp ){
		// sending a signal that it is no file with that name present.
		output_file_not_found(socket);
		return;
	}

	const char * file_type = path;
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

void interpret_and_output(int socket, char * client_ip, char * time)
{

	char *c, *args, *command, *path, *cleaner;
	int arg_count, found, n;
	unsigned long count;
	hashtable_t * params;

	char first_line[BACKEND_MAX_ARRAY_SIZE], *buffer = malloc(BACKEND_MAX_BUFFER_SIZE); 

	memset(buffer, '\0', BACKEND_MAX_BUFFER_SIZE);
	n = read(socket,buffer,BACKEND_MAX_BUFFER_SIZE);

	// Getting the first line of the input

	c = buffer;
	found = 0;
	count = 0;
	while (*c && !found){ if ( *c == '\n'){ found = 1; } ++c; ++count;}
	memset(first_line, '\0', sizeof(first_line));
	strncpy(first_line, buffer, count);

	first_line[sizeof(first_line) - 1] = '\0';

	c = first_line;
	while (*c) {
		if ( *c == '\n'){
			*c = '\0';
			break;
		}
		++c;
	}

	log("[%s] %s: %s\n", time, client_ip, first_line);

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

		if ( strstr(path, "coffe.cgi") != NULL ){
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

			free_hashtable(params);
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

			free_hashtable(params);
		} else {
			output_path(socket, &path[1]);
		}

	} else if ( !strcmp(command, "POST") ){

	}

	free(buffer);
	
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

	int socket = (int) *http_data->socket;
	char *client_ip = http_data->client_ip, *time = http_data->accept_time;

	// look at the first line of 'buffer' and do what you got to do..
	interpret_and_output(socket, client_ip, time);
   
   	close(socket);

   	free_http_data(&http_data);

   	return NULL;
}