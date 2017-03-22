#include "cgi.h"

/*
* Will translate the headers in the HTTP request
* to os environment variables in a python file 
*/
void cgi_py(int socket, hashtable_t* params, const char * cgi_path)
{
	FILE *fp,*process;
	char *extension,buffer[1024];

	memset(buffer, '\0', sizeof(buffer));

	fp = fopen("cgi/python.cgi", "w");
	fprintf(fp, "import os\n");

	write_table_as_python_os_environ(params, fp);

	// Important: the cgi_path has already been verified that it exists
	// and that it has the extension .py

	extension = strstr(cgi_path, ".py");
	*extension = '\0';

	fprintf(fp, "import %s", strstr(cgi_path+1,"/")+1);

	fclose(fp);

	process = popen("python cgi/python.cgi", "r");

	while ( fgets(buffer, sizeof(buffer)-1, process) != NULL ){
		write(socket, buffer, strlen(buffer));
	}

	fclose(process);
}


void cgi_sh(int socket, hashtable_t* params, const char * cgi_path)
{
	FILE *fp,*process;
	char *extension,buffer[1024];

	memset(buffer, '\0', sizeof(buffer));

	fp = fopen("cgi/bash.cgi", "w");
	fprintf(fp,"#!/bin/sh\n");

	write_table_as_bash_variables(params, fp);
	// Important: the cgi_path has already been verified that it exists
	// and that it has the extension .sh

	extension = strstr(cgi_path, ".sh");
	*extension = '\0';

	fprintf(fp, ". cgi/%s.sh", strstr(cgi_path+1,"/")+1);

	fclose(fp);

	system("chmod ugo+x cgi/bash.cgi");			
	process = popen("./cgi/bash.cgi", "r");

	while ( fgets(buffer, sizeof(buffer), process) != NULL ){
		write(socket, buffer, strlen(buffer));
	}

	fclose(process);
}