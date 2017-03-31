#include "file_display_cgi.h"

void file_display_cgi(int socket, hashtable_t * params)
{
	char *action, *file, *data, cmd[200];
	int fd;
	unsigned sz;
	FILE * pd;
	struct stat st;

	action = (char*) get(params, "action");
	file = (char*) get(params, "path");

	if ( !strcmp(action, "list") ){

		memset(cmd, '\0', sizeof cmd);

		snprintf(cmd, sizeof(cmd)-1, "ls -la %s", file);

		pd = popen(cmd, "r");

		if ( pd == NULL ){
			output_file_not_found(socket);
			return;
		}

		output_txt_headers(socket);

		while ( fgets(cmd, sizeof(cmd) - 1, pd) != NULL ){
			write(socket, cmd, strlen(cmd));
			memset(cmd, '\0', sizeof(cmd) - 1);
		} 

		pclose(pd);

	} else if ( !strcmp(action, "download") ){

		fd = open(file, O_RDONLY);

		if ( fd < 0 ) {
			output_file_not_found(socket);
			return;
		}

		stat(file, &st);
		sz = st.st_size;

		output_file_transfer_headers(socket, file);

		data = malloc(sz+1);
		if ( data == NULL ){
			output_internal_server_error(socket);
			return;
		}

		data[sz] = '\0';

		read(fd, data, sz);
		write(socket, data, sz);

		free(data);
		close(fd);

	} else {

		output_file_not_found(socket);

	}




}