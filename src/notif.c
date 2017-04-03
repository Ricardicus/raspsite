#include "notif.h"

/*
* To shift endianness of received or transmitted bytes
*/
static void swap(const void *src, void *dst, size_t sz)
{
	size_t i = 0;
	for (; i<sz; ++i){
		((char*)dst)[i] = ((char*)src)[sz-1-i];
	}
}

int make_contact(const char * host, int port)
{
	struct sockaddr_in serv_addr;
	struct hostent *server;
	struct timeval tv;
	int sockfd;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if ( sockfd < 0 ) {
		log_error("Failed to make contact, can't for whatever reason not create a socket!\n");
		return -1;
	}

	/* Setting timeout on receive operations for this socket */
	tv.tv_sec = 60;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));

	server = gethostbyname(host);
	if ( server == NULL ){
		log_error("Failed to make contact, can't resolve host: %s\n", host);
		return -1;
	}

	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*) server->h_addr,
		(char*) &serv_addr.sin_addr.s_addr,
		server->h_length);
	serv_addr.sin_port = htons(port);

	if (connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0){
		log_error("Failed to make contact, can't connect to host: %s\n", host);
		return -1;
	}

	return sockfd;
}

/*
* 	Protocol uses big-endian byte representation on all fields
*	
* 	File sending header format: 
*			| type | data length |<-- length -->| name length |<-- name -->|<--- data ---->| 
*	Bytes:		
*				1	  	  1			data length		  1 	   name length	 	length
*								     [1,2,4,8]
*
*	File sending response:
*			| type |
*	Bytes:
*			   1
*/
int output_file(int socket, const char * filepath, char * filename)
{
	FILE * fp;
	long int file_size;
	int data;
	char command_ch, size_length_ch, response_ch, 
			name_length_ch;

	bool is_little_endian = false;

	{
		int n = 1;
		is_little_endian = (*((char*)&n) == 1);
	}

	if ( strlen(filename) > 255 ){
		printf("Filename to long. Cannot exceed one byte.");
		return -1;
	}

	fp = fopen(filepath, "r");
	
	if ( fp == NULL ){
		log_error("Failed to send file: %s, file not found.", filename);
		return -1;
	}

	// Telling the server that I'm here to send a file
	command_ch = SEND_FILE;
	write(socket, &command_ch, 1);

	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	// Telling the server how many bytes are required to represent the file size
	size_length_ch = sizeof(file_size);

	write(socket, &size_length_ch, 1);

	// Telling the server just how many bytes are expected to be sent
	if ( is_little_endian ) {
		// Swapping to big endian
		long int tmp;
		swap(&file_size, &tmp, sizeof file_size );
		file_size = tmp;
	}

	write(socket, &file_size, size_length_ch);

	if ( is_little_endian ) {
		// Swapping back to little endian
		long int tmp;
		swap(&file_size, &tmp, sizeof file_size );
		file_size = tmp;
	}


	name_length_ch = strlen(filename);
	printf("filename is of length: %d\n", name_length_ch);
	// telling the server how long the name will be
	write(socket, &name_length_ch, 1);
	// writing the filename
	write(socket,filename, name_length_ch);

	// sending the bytes of the file!
	while ( (data=fgetc(fp)) != EOF) {
		write(socket, &data, 1);
	}

	response_ch = TIMEOUT;

	read(socket, &response_ch, 1);

	switch (response_ch) {
	case EVERYTHING_OK:
		printf("File: \"%s\" of size: %ld sent to server.\n", filename, file_size);
	break;
	case SOMETHING_WRONG:
		log_error("Server not happy after send operation.");
	break;
	case RECEIVED_OVERWRITE:
		log("File: \"%s\" of size: %ld sent to server, previous file was overwritten.", filename, file_size);
	break;
	default:
		log_error("Server answered with alien response after send operation.");
	break;
	}

	return response_ch;
}

void receive_file(int socket, const char * directory){
	char buffer[257], filepath[457],response_ch;
	FILE * fp;

	int8_t size_8 = 0;
	int16_t size_16 = 0;
	int32_t size_32 = 0;
	int64_t size_64 = 0;

	bool is_little_endian;

	{
		int n = 1;
		is_little_endian = (*((char*)&n) == 1);
	}


	memset(buffer, '\0', 257);

	if ( strlen(directory) >= 200 ){
		log_error("Directory name too long!\n");
		return;
	}

	// Getting the filesize of the data type representing the data
	// to be sent.  
	read(socket, &response_ch, 1);
	switch(response_ch) {
	case 1:
		read(socket, &size_8, 1);
		printf("The file size is: %c\n", size_8);
	break;
	case 2:
		read(socket, &size_16, 2);
		if ( is_little_endian ) {
			int16_t tmp;
			swap(&size_16, &tmp, sizeof size_16);
			size_16 = tmp;	
		}
		printf("The file size is: %u\n", size_16);
	break;
	case 4:
		read(socket, &size_32, 4);
		if ( is_little_endian ) {
			int32_t tmp;
			swap(&size_32, &tmp, sizeof size_32);
			size_32 = tmp;	
		}
		printf("The file size is: %u\n", size_32);
	break;
	case 8:
		read(socket, &size_64, 8);
		if ( is_little_endian ) {
			int32_t tmp;
			swap(&size_64, &tmp, sizeof size_64);
			size_64 = tmp;	
		}
		printf("The file size is: %llu\n", size_64);
	break;
	default:
		printf("Alien file size type received\n");
		return;
	break;
	}

	// Getting the size of the wanted file name
	read(socket, &response_ch, 1);

	// Getting the name of the file
	read(socket, &buffer, (int)response_ch);	

	printf("Filename: %s\n", buffer);

	memset(filepath, '\0', sizeof(filepath));
	sprintf(filepath, "%s/%s", directory, buffer);

	fp = fopen(filepath, "w");

	// Writing the data to file
	if ( size_8 != 0 ){

		int8_t i = 0;

		for (; i < size_8; ++i ){
			read(socket, &response_ch, 1);
			fputc(response_ch, fp);
		}

		fclose(fp);

		response_ch = EVERYTHING_OK;
		write(socket, &response_ch, 1);
		printf("wrote everything ok\n");

		close(socket);

		printf("File: %s of %d bytes successfully received.", buffer, size_8);

	} else if ( size_16 != 0 ){

		int16_t i = 0;

		for (; i < size_16; ++i ){
			read(socket, &response_ch, 1);
			fputc(response_ch, fp);
		}
		fclose(fp);

		response_ch = EVERYTHING_OK;

		write(socket, &response_ch, 1);

		close(socket);
		printf("File: %s of %d bytes successfully received.\n", buffer, size_16);

	} else if ( size_32 != 0 ){

		int32_t i = 0;

		for (; i < size_32; ++i ){
			printf("size_32 read [i: %d]\n", i);
			read(socket, &response_ch, 1);
			fputc(response_ch, fp);
		}
		fclose(fp);

		response_ch = EVERYTHING_OK;
		write(socket, &response_ch, 1);

		close(socket);
		printf("File: %s of %d bytes successfully received.\n", buffer, size_32);
	
	} else if ( size_64 != 0 ){

		int64_t  i=0;

		for (; i < size_64; i++ ){
			read(socket, &response_ch, 1);
			fputc(response_ch, fp);
		}
		fclose(fp);

		response_ch = EVERYTHING_OK;
		write(socket, &response_ch, 1);

		close(socket);
		printf("File: %s of %llu bytes successfully received.\n", buffer, size_64);

	}

}

void output_greeting(int socket)
{

}

void output_alternatives(int socket)
{

}
