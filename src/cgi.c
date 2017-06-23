#include "cgi.h"

/*
* Will translate the headers in the HTTP request
* to os environment variables in a python file 
*/
void cgi_py(int socket, hashtable_t* params, const char * cgi_path)
{
	FILE *fp;
	char *extension,buffer[1024], *file_name = "cgi/python.cgi";
	static char * path = NULL;
	pid_t pid;

	memset(buffer, '\0', sizeof(buffer));

	fp = fopen(file_name, "w");
	if ( fp == NULL ) {
		printf("fp is null\n");
		return;
	}

	if ( path == NULL )
		path = traverse_path_to_find("python");

	if ( path == NULL ) {
		log_error("Python not found on machine.\n");
		return;
	}

	fprintf(fp, "#!%s\nimport os\n", path);

	write_table_as_python_os_environ(params, fp);

	// Important: the cgi_path has already been verified that it exists
	// and that it has the extension .py

	extension = strstr(cgi_path, ".py");
	*extension = '\0';

	fprintf(fp, "import %s", strstr(cgi_path,"/")+1);

	fclose(fp);
	
	chmod(file_name, S_IXUSR | S_IXGRP | S_IXOTH \
		| S_IWUSR | S_IWGRP | S_IWOTH | S_IRUSR | \
			S_IRGRP | S_IROTH ); // Anyone may do whatever with this file

	pid = fork();

	if ( pid < 0 )
		return;

	if ( pid == 0 ) {

		char *args[1]; // No arguments needed here. All arguments are written to the file.
		args[0] = NULL;

		char full_path_buff[2148];
		memset(full_path_buff,'\0', sizeof full_path_buff);
		char cwd[2048];
		memset(cwd,'\0', sizeof cwd);

		getcwd(cwd,sizeof cwd);

		snprintf(full_path_buff, sizeof(full_path_buff) - 1, "%s/%s", cwd, file_name);

		dup2(socket, STDOUT_FILENO);

		errno = 0;

		execv(full_path_buff, args);
		int error = errno;
		log_error("Exevc failed for file_name: %s, errno: %d\n", file_name, error);

#if 0
		switch(error){
			case E2BIG:
			log_error("E2BIG\n");
			break;
			case EACCESS:
			log_error("EACCESS\n");
			break;
			case EAGAIN:
			log_error("EAGAIN\n");
			break;
			case EFAULT:
			log_error("EFAULT\n");
			break;
			case EINVAL:
			log_error("EINVAL\n");
			break;
			case EIO:
			log_error("EIO\n");
			break;
			case EISDIR:
			log_error("EISDIR\n");
			break;
			case ELIBBAD:
			log_error("ELIBBAD\n");
			break;
			case ELOOP:
			log_error("ELOOP\n");
			break;
			case EMFILE:
			log_error("EMFILE\n");
			break;
			case ENAMETOOLONG:
			log_error("ENAMETOOLONG\n");
			break;
			case ENFILE:
			log_error("ENFILE\n");
			break;
			case ENOENT:
			log_error("ENOENT\n");
			break;
			case ENOEXEC:
			log_error("ENOEXEC\n");
			break;
			case ENOMEM:
			log_error("ENOMEM\n");
			break;
			case ENOTDIR:
			log_error("ENOTDIR\n");
			break;
			case EPERM:
			log_error("EPERM\n");
			break;
			case EXTBSY:
			log_error("EXTBSY\n");
			break;
		}
#endif 

	} else if ( pid > 0 ) {

		int status;
		waitpid(pid, &status, 0);
		close(socket);

	}

}


void cgi_sh(int socket, hashtable_t* params, const char * cgi_path)
{
	FILE *fp;
	char *extension,buffer[1024], *file_name = "cgi/bash.cgi";
	static char * path;
	pid_t pid;

	memset(buffer, '\0', sizeof(buffer));

	fp = fopen(file_name, "w");
	fprintf(fp,"#!/bin/sh\n");

	if ( path == NULL )
		path = traverse_path_to_find("sh");

	if ( path == NULL ) {
		log_error("Shell not found on machine.\n");
		return;
	}

	write_table_as_bash_variables(params, fp);
	// Important: the cgi_path has already been verified that it exists
	// and that it has the extension .sh

	extension = strstr(cgi_path, ".sh");
	*extension = '\0';

	fprintf(fp, ". cgi/%s.sh", strstr(cgi_path,"/")+1);

	fclose(fp);
	
	chmod(file_name, S_IXUSR | S_IXGRP | S_IXOTH \
		| S_IWUSR | S_IWGRP | S_IWOTH | S_IRUSR | \
			S_IRGRP | S_IROTH ); // Anyone may do whatever with this file

	pid = fork();

	if ( pid < 0 )
		return;

	if ( pid == 0 ) {

		char *args[1]; // No arguments needed here. All arguments are written to the file.
		args[0] = NULL;

		char full_path_buff[2148];
		memset(full_path_buff,'\0', sizeof full_path_buff);
		char cwd[2048];
		memset(cwd,'\0', sizeof cwd);

		getcwd(cwd,sizeof cwd);

		snprintf(full_path_buff, sizeof(full_path_buff) - 1, "%s/%s", cwd, file_name);

		dup2(socket, STDOUT_FILENO);

		errno = 0;

		execv(full_path_buff, args);
		int error = errno;
		log_error("Exevc failed for file_name: %s, errno: %d\n", file_name, error);

	} else if ( pid > 0 ) {

		int status;
		waitpid(pid, &status, 0);
		close(socket);

	}
}

char* traverse_path_to_find(const char * file)
{
	const char * full_path = getenv("PATH");
	char * path_buffer = calloc(2048, 1), *c_s, *c_e, *result;

	snprintf(path_buffer, 2047, "%s:", full_path);

	c_s = path_buffer;
	while ( (c_e = strchr(c_s, ':')) != NULL ) {
		*c_e = '\0';

		result = calloc(strlen(c_s) + strlen(file) + 2,1);
		sprintf(result, "%s/%s", c_s, file);

		if ( !access(result, X_OK) ){
			free(path_buffer);
			printf("found: %s\n", result);
			return result;
		}

		c_s = c_e+1;

		free(result);
	}

	free(path_buffer);

	return NULL;

}