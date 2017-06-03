#include "sec_session.h"

static char *server_responses[] = (char *[]) { 
	"Welcome!\nFor help enter 'help'.\n", // Greeting
	"These are your alternatives: \n\
	* 'help': display this tutorial.\n\
	* 'shell': enter the shell over this 'secure' channel.\n \
	* 'quit': exit this session.\n-- More to come (possibly) --\n",
	"Could not interpret that command. For help enter 'help'.\n",
	"Enter user credentials. You need to authenticate yourself.\n"
};

/*
* To shift endianness of received or transmitted bytes
*/
static void swap(const void *src, void *dst, size_t sz)
{
	size_t i = 0;
	for (; i<sz; ++i){
		((unsigned char*)dst)[i] = ((unsigned char*)src)[sz-1-i];
	}
}

static void initialize_session(rsa_session_t * session, void * symmetric_key, size_t key_size)
{
	// Generate the RSA keys
	generate_rsa_keys(session);	

	// Generate the random symmetric key
	generate_symmetric_key(symmetric_key, key_size);
}

/*
* 			Secure Communication Protocol 
* ======================================================
*
* The communication channel is about to be established
* after all the encryption parameters have been sent.
*
* 1. 	Client sends its rsa encryption coefficients:
*		| RSA N | RSA E | 
* Bytes:
*			8		8
* 
* 2. 	Server responds with its rsa encryption coefficients as well as 
		its symmetric key encrypted using the clients rsa coefficient:
*		| RSA N | RSA E | KEY LEN (encrypted) |  KEY (encrypted) 	|
* Bytes:
*			8		8		8					KEY LEN * 8
* 3. 	Client sends its symmetric key encrypted using the servers 
*		rsa coefficients:
*		| KEY LEN (encrypted)| KEY (encrypted) |
* Bytes:
*			8		KEY LEN 
* 
* * 	Client/Server protocol from now on will look like this:				
*		| MSG LEN | 	MSG	  |
*			 8		 MSG LEN	
*
* All messages are encrypted with its respective decrypted symmetric key 	
* using the function 'symmetric_key_crypto' in "rsa.c".
*
* =======================================================
*/
void sec_session_server(int socket, const char* client_ip)
{
	unsigned char symmetric_key_here[STNDRD_SYM_KEY_LEN], *key_len_decode_buffer,
			*sym_key_over_tcp, *sym_key_of_peer, *key_len_over_tcp, peer_key_len_over_tcp[8],
				read_write_buffer[MAX_DATA_LEN];
	char server_talk[MAX_DATA_LEN + KEY_PADDING + 1], oldpwd[200];
	rsa_session_t private_session, public_session; // in the public only n and e are known.
	uint64_t n_over_tcp, e_over_tcp, key_len, peer_sym_key_len,
				sz_send, msg_len;
	int ctrl;

	bool is_little_endian = false, shell_activated = false, is_authenticated = false;

	{
		int n = 1;
		is_little_endian = (*((char*)&n) == 1);
	}

	initialize_session(&private_session, symmetric_key_here, sizeof symmetric_key_here); // this process might take some time (not to long though)

	// Getting the RSA public keys of the public session
	memset(read_write_buffer, '\0', MAX_DATA_LEN);
	ctrl = read(socket, read_write_buffer, 2*KEY_PADDING);

	if ( ctrl != 2*KEY_PADDING ) {
		log_error("Attempted to read %d bytes, got %d.\n", 2*KEY_PADDING, ctrl);
		close(socket);
		return;
	}

	memcpy(&n_over_tcp, read_write_buffer, KEY_PADDING);
	memcpy(&e_over_tcp, read_write_buffer + KEY_PADDING, KEY_PADDING);

	if ( is_little_endian == true ) {
		// making all the RSA coefficients appear over TCP in big endian order

		swap(&n_over_tcp, &public_session.n, sizeof public_session.n );
		swap(&e_over_tcp, &public_session.e, sizeof public_session.e );

	} else {

		public_session.n = n_over_tcp;
		public_session.e = e_over_tcp;
		
	}

	if ( is_little_endian == true ) {
		// making all the RSA coefficients appear over TCP in big endian order
		swap(&private_session.n, &n_over_tcp, sizeof private_session.n );
		swap(&private_session.e, &e_over_tcp, sizeof private_session.e );

	} else {

		n_over_tcp = private_session.n;
		e_over_tcp = private_session.e;
		
	}

	// Sending the RSA public keys of the private session
	memset(read_write_buffer, '\0', sizeof read_write_buffer);
	memcpy(read_write_buffer, &n_over_tcp, KEY_PADDING);
	memcpy(read_write_buffer + KEY_PADDING, &e_over_tcp, KEY_PADDING);

	ctrl = write(socket, read_write_buffer, 2 * KEY_PADDING);

	if ( ctrl != 2 * KEY_PADDING ) {
		log_error("Attempted to write %d bytes, wrote %d.\n", 2 * KEY_PADDING, ctrl);
		close(socket);
		return;
	}

	key_len = KEY_PADDING * sizeof(symmetric_key_here);

	key_len_over_tcp = rsa_encode(&public_session, is_little_endian ? &((unsigned char*)&key_len)[0] : &((unsigned char*)&key_len)[KEY_PADDING-1], 1);

	// writing the encoded value of the expected length of the symmetric key
	ctrl = write(socket, key_len_over_tcp, KEY_PADDING);
	free(key_len_over_tcp);

	if ( ctrl != KEY_PADDING ) {
		log_error("Attempted to write %d bytes, wrote %d.\n", KEY_PADDING, ctrl);
		close(socket);
		return;
	}


	// writing the encoded version of the symmetric key
	sym_key_over_tcp = rsa_encode(&public_session, symmetric_key_here , sizeof symmetric_key_here );
	ctrl = write(socket, sym_key_over_tcp, key_len);
	free(sym_key_over_tcp);

	if ( ctrl != key_len ) {
		log_error("Attempted to write %llu bytes, wrote %d.\n", key_len, ctrl);
		close(socket);
		return;
	}

	// Reading the encrypted value of the length of the symmetric key
	ctrl = read(socket, peer_key_len_over_tcp, KEY_PADDING);

	if ( ctrl != KEY_PADDING ) {
		log_error("Attempted to read %d bytes, got %d.\n", KEY_PADDING, ctrl);
		close(socket);
		return;
	}

	// Decoding the key length
	key_len_decode_buffer = rsa_decode(&private_session, peer_key_len_over_tcp, sizeof peer_key_len_over_tcp ); // might take some time

	key_len = 0, key_len += key_len_decode_buffer[0]; // the decoded key length is only one char long
	free(key_len_decode_buffer); // its also heap allocated (consequence of current 'rsa_decode' implementation)

	if ( key_len > MAX_SYM_KEY_LEN ) {
		printf("Read key length: %" PRIu64 " too large. Session interrupted.\n", key_len);
		close(socket);
		return; 
	}

	sym_key_over_tcp = (unsigned char*) malloc(key_len); // allocating space for the encrypted symemtric key to be read

	peer_sym_key_len = key_len / KEY_PADDING;
	ctrl = read(socket, sym_key_over_tcp, key_len); // reading the encrypted symmetric key used by the other peer

	if ( ctrl != key_len ) {
		log_error("Attempted to read %llu bytes, got %d.\n", key_len, ctrl);
		free(sym_key_over_tcp);
		close(socket);
		return;
	}

	printf("Decoding (Might take time)\n");

	// Decoding the symmetric key of the client
	sym_key_of_peer = rsa_decode(&private_session, sym_key_over_tcp, key_len); // will take some time..
	free(sym_key_over_tcp); // freeing this now

	printf("Decoded!\n");

	/*
	* All parameters are transferred and the secure connection is established.
	* ........... 				More to come next! 			................
	*/

	log("Connection initalized by client is established!\n");

	// Start by outputting the greeting
	msg_len = (uint64_t) strlen(server_responses[AUTHENTICATE]), sz_send = msg_len;

	if ( is_little_endian ) {
		uint64_t tmp;
		swap(&sz_send, &tmp, 8);
		sz_send = tmp;
	}

	memcpy(server_talk, &sz_send, KEY_PADDING);
	memcpy(server_talk + KEY_PADDING, server_responses[AUTHENTICATE], msg_len);

	symmetric_key_crypto(symmetric_key_here, sizeof symmetric_key_here, server_talk, msg_len + 8, ENCRYPT);

	ctrl = write(socket, server_talk, msg_len + KEY_PADDING); // writing the encrypted version of the greeting

	if ( ctrl != msg_len + KEY_PADDING ) {
		log_error("Attempted to write %llu bytes, wrote %d.\n", msg_len + KEY_PADDING , ctrl);
		free(sym_key_of_peer);
		close(socket);
		return;
	}

	/*
	* More to come from here on out! :) 
	* We have established the secure communication channel!
	*/

	while ( 1 ) {

		memset(server_talk, '\0', sizeof server_talk);

		ctrl = read(socket, server_talk, KEY_PADDING); // Get the encrypted length of the message to be passed..

		if ( ctrl == 0 ) {
			printf("[%s].%d Interrupted session!\n", __func__, __LINE__);
			free(sym_key_of_peer);

			return;
		} else if ( ctrl != KEY_PADDING ) {
			printf("[%s].%d Interrupted session!\n", __func__, __LINE__);
			free(sym_key_of_peer);
			return;
		}

		symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, KEY_PADDING, DECRYPT);

		if ( is_little_endian ){
			uint64_t tmp = 0;
			swap(server_talk, &tmp, KEY_PADDING);
			sz_send = tmp;
		} else {
			uint64_t tmp = 0;
			memcpy(&tmp, server_talk, KEY_PADDING);
			sz_send = tmp;
		}

		if ( sz_send > MAX_DATA_LEN ){
			free(sym_key_of_peer);
			close(socket);
			printf("[%s] ERROR AT: %d\n", __func__, __LINE__);
			return;
		}

		symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, KEY_PADDING, ENCRYPT);

		ctrl = read(socket, server_talk + KEY_PADDING, sz_send);

		if ( ctrl != sz_send ) {
			log_error("Attempted to read %" PRIu64 " bytes, got %d.\n", sz_send, ctrl);
			free(sym_key_of_peer);
			close(socket);
			return;
		}

		symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, sz_send + KEY_PADDING, DECRYPT);	

		printf("received the message: %s", server_talk + KEY_PADDING);

		if ( is_authenticated == false ) {

			if ( strstr(server_talk + KEY_PADDING, STANDARD_USER_PASSWORD ) == NULL ){
				// Output the need for authentication
				memset(server_talk, '\0', sizeof server_talk);

				msg_len = (uint64_t) strlen(server_responses[AUTHENTICATE]), sz_send = msg_len;

				if ( is_little_endian ) {
					uint64_t tmp;
					swap(&sz_send, &tmp, 8);
					sz_send = tmp;
				}

				memcpy(server_talk, &sz_send, KEY_PADDING);
				memcpy(server_talk + KEY_PADDING, server_responses[AUTHENTICATE], msg_len);

				symmetric_key_crypto(symmetric_key_here, sizeof symmetric_key_here, server_talk, msg_len + 8, ENCRYPT);

				ctrl = write(socket, server_talk, msg_len + KEY_PADDING); // writing the encrypted version of the greeting

				if ( ctrl != msg_len + KEY_PADDING ) {
					log_error("Attempted to write %" PRIu64 " bytes, wrote %d.\n", msg_len + KEY_PADDING , ctrl);
					free(sym_key_of_peer);
					close(socket);
					return;
				}
			} else {
				// Output the greeting
				memset(server_talk, '\0', sizeof server_talk);

				msg_len = (uint64_t) strlen(server_responses[GREET]), sz_send = msg_len;

				if ( is_little_endian ) {
					uint64_t tmp;
					swap(&sz_send, &tmp, 8);
					sz_send = tmp;
				}

				memcpy(server_talk, &sz_send, KEY_PADDING);
				memcpy(server_talk + KEY_PADDING, server_responses[GREET], msg_len);

				symmetric_key_crypto(symmetric_key_here, sizeof symmetric_key_here, server_talk, msg_len + 8, ENCRYPT);

				ctrl = write(socket, server_talk, msg_len + KEY_PADDING); // writing the encrypted version of the greeting

				if ( ctrl != msg_len + KEY_PADDING ) {
					log_error("Attempted to write %" PRIu64 " bytes, wrote %d.\n", msg_len + KEY_PADDING , ctrl);
					free(sym_key_of_peer);
					close(socket);
					return;
				}

				is_authenticated = true;

				send_mail( GMAIL_LOGGER, GMAIL_TO_NOTIFY, "Authentication for shell access on my RaspberryPi", AUTH_SUCCESS_MAIL, client_ip);

			}



		} else if ( shell_activated == false ) {
			// The course of action when not in shell mode

			if ( strstr(server_talk + KEY_PADDING, "help") != NULL ) {
				// Output the alternatives
				memset(server_talk, '\0', sizeof server_talk);

				msg_len = (uint64_t) strlen(server_responses[ALTERNATIVES]), sz_send = msg_len;

				if ( is_little_endian ) {
					uint64_t tmp;
					swap(&sz_send, &tmp, 8);
					sz_send = tmp;
				}

				memcpy(server_talk, &sz_send, KEY_PADDING);
				memcpy(server_talk + KEY_PADDING, server_responses[ALTERNATIVES], msg_len);

				symmetric_key_crypto(symmetric_key_here, sizeof symmetric_key_here, server_talk, msg_len + 8, ENCRYPT);

				ctrl = write(socket, server_talk, msg_len + KEY_PADDING); // writing the encrypted version of the greeting

				if ( ctrl != msg_len + KEY_PADDING ) {
					log_error("Attempted to write %" PRIu64 " bytes, wrote %d.\n", msg_len + KEY_PADDING , ctrl);
					free(sym_key_of_peer);
					close(socket);
					return;
				}

			} else if ( strstr(server_talk + KEY_PADDING, "shell") != NULL ) {
				// Output the alternatives
				memset(server_talk, '\0', sizeof server_talk);
				char * msg = " ========== SHELL MODE ACTIVATED ('quit' still works) !! =========\n";

				msg_len = (uint64_t) strlen(msg), sz_send = msg_len;

				if ( is_little_endian ) {
					uint64_t tmp;
					swap(&sz_send, &tmp, 8);
					sz_send = tmp;
				}

				memcpy(server_talk, &sz_send, KEY_PADDING);
				memcpy(server_talk + KEY_PADDING, msg, msg_len);

				symmetric_key_crypto(symmetric_key_here, sizeof symmetric_key_here, server_talk, msg_len + 8, ENCRYPT);

				ctrl = write(socket, server_talk, msg_len + KEY_PADDING); // writing the encrypted version of the greeting

				if ( ctrl != msg_len + KEY_PADDING ) {
					log_error("Attempted to write %" PRIu64 " bytes, wrote %d.\n", msg_len + KEY_PADDING , ctrl);
					free(sym_key_of_peer);
					close(socket);
					return;
				}

				shell_activated = true;

			} else {
				// Failed to interpret the input from the client the alternatives
				memset(server_talk, '\0', sizeof server_talk);

				msg_len = (uint64_t) strlen(server_responses[MISUNDERSTOOD]), sz_send = msg_len;

				if ( is_little_endian ) {
					uint64_t tmp;
					swap(&sz_send, &tmp, 8);
					sz_send = tmp;
				}

				memcpy(server_talk, &sz_send, KEY_PADDING);
				memcpy(server_talk + KEY_PADDING, server_responses[MISUNDERSTOOD], msg_len);

				symmetric_key_crypto(symmetric_key_here, sizeof symmetric_key_here, server_talk, msg_len + 8, ENCRYPT);

				ctrl = write(socket, server_talk, msg_len + KEY_PADDING); // writing the encrypted version of the greeting

				if ( ctrl != msg_len + KEY_PADDING ) {
					log_error("Attempted to write %" PRIu64 " bytes, wrote %d.\n", msg_len + KEY_PADDING , ctrl);
					free(sym_key_of_peer);
					close(socket);
					return;
				}
			}
		} else {

			if ( !strncmp(server_talk + KEY_PADDING, "cd ", 3) ||!strncmp(server_talk + KEY_PADDING, "cd\n", 3) ) {
				/* This will be a unique case, since cd is no binary (shell built in) */
				memset(oldpwd, '\0', sizeof oldpwd);

				int argc = 1, cd_error=0;
				char *search, *c;

				c = strchr(server_talk + KEY_PADDING, '\n');

				if ( c != NULL )
					*c = '\0';

				c = strchr(server_talk+KEY_PADDING, ' ');
				search = c;

				if (search != NULL ){
					while( *search == ' ' )
						++search;
					if ( *search ) {
						// There are two arguments
						argc = 2;
					}
				}

				if ( argc == 1 ){
					snprintf(oldpwd, sizeof oldpwd, "%s", getenv("PWD"));
					char * home = getenv("HOME");
					chdir(home);
					setenv("PWD", home, 1);
					setenv("OLDPWD", oldpwd, 1);
				} else if ( argc > 1 && strncmp(search, "-", 1) == 0) {
					char *pwd = getenv("PWD");
					snprintf(oldpwd, sizeof oldpwd, "%s", getenv("OLDPWD"));
			//		printf("Attempting to change to: %s\n", oldpwd);
					if (chdir(oldpwd)) {
						cd_error = 1;
					}
					setenv("OLDPWD", pwd, 1);
					setenv("PWD", oldpwd, 1);
					printf("%s\n", oldpwd);
				} else if ( argc > 1 && chdir(search) == 0) {
					setenv("OLDPWD", getenv("PWD"), 1);
					char pwd_string[MAX_DATA_LEN];
					memset(pwd_string, 0, sizeof pwd_string);
					getcwd(pwd_string, MAX_DATA_LEN-1);
					setenv("PWD", pwd_string, 1);
				} else {
					cd_error = 1;
				}
				
				if ( cd_error == 1 ){
					// Failed to interpret the input from the client the alternatives
					memset(server_talk, '\0', sizeof server_talk);

					char *msg = "error on cd: could not change directory.\n";
					msg_len = (uint64_t) strlen(msg), sz_send = msg_len;

					if ( is_little_endian ) {
						uint64_t tmp;
						swap(&sz_send, &tmp, 8);
						sz_send = tmp;
					}

					memcpy(server_talk, &sz_send, KEY_PADDING);
					memcpy(server_talk + KEY_PADDING, msg, msg_len);

					symmetric_key_crypto(symmetric_key_here, sizeof symmetric_key_here, server_talk, msg_len + KEY_PADDING, ENCRYPT);

					ctrl = write(socket, server_talk, msg_len + KEY_PADDING); // writing the encrypted version of the greeting

					if ( ctrl != msg_len + KEY_PADDING ) {
						log_error("Attempted to write %" PRIu64 " bytes, wrote %d.\n", msg_len + KEY_PADDING , ctrl);
						free(sym_key_of_peer);
						close(socket);
						return;
					}

				} else {
					// Failed to interpret the input from the client the alternatives
					memset(server_talk, '\0', sizeof server_talk);

					char *msg = getenv("PWD");

					msg_len = (uint64_t) strlen(msg) + 1, sz_send = msg_len; // add one for line feed

					if ( is_little_endian ) {
						uint64_t tmp;
						swap(&sz_send, &tmp, 8);
						sz_send = tmp;
					}

					memcpy(server_talk, &sz_send, KEY_PADDING);
					sprintf(server_talk + KEY_PADDING, "%s\n", msg);

					symmetric_key_crypto(symmetric_key_here, sizeof symmetric_key_here, server_talk, msg_len + KEY_PADDING, ENCRYPT);

					ctrl = write(socket, server_talk, msg_len + KEY_PADDING); // writing the encrypted version of the greeting

					if ( ctrl != msg_len + KEY_PADDING) {
						log_error("Attempted to write %" PRIu64 " bytes, wrote %d.\n", msg_len + KEY_PADDING , ctrl);
						free(sym_key_of_peer);
						close(socket);
						return;
					}

				}

			} else {
				// Will output whatever comes from the command in popen!!!
				char output_from_command[MAX_DATA_LEN+1], output_buffer[MAX_DATA_LEN+1], *c;
				int status;
				size_t count = 0, stored = 0, output_len;
				FILE *process;

				memset(output_buffer, '\0', sizeof output_buffer);
				memset(output_from_command, '\0', sizeof output_from_command);

				c = strchr(server_talk + KEY_PADDING, '\n');
				
				if ( c != NULL )
					*c = '\0';

				process = popen(server_talk + KEY_PADDING, "r");

				while ( fgets(output_buffer, MAX_DATA_LEN, process) != NULL ){

					output_len = strlen(output_buffer);
					count = 0;

					if ( stored + output_len >= MAX_DATA_LEN ){

						while ( stored < MAX_DATA_LEN ){
							output_from_command[stored++] = output_buffer[count++];
						}

						msg_len = (uint64_t) strlen(output_from_command), sz_send = msg_len;

						if ( is_little_endian ) {
							uint64_t tmp;
							swap(&sz_send, &tmp, 8);
							sz_send = tmp;
						}

						memcpy(server_talk, &sz_send, KEY_PADDING);
						sprintf(server_talk + KEY_PADDING, "%s", output_from_command);

						symmetric_key_crypto(symmetric_key_here, sizeof symmetric_key_here, server_talk, msg_len + KEY_PADDING, ENCRYPT);

						ctrl = write(socket, server_talk, msg_len + KEY_PADDING); // writing the encrypted version of the greeting

						if ( ctrl != msg_len + KEY_PADDING ) {
							log_error("Attempted to write %" PRIu64 " bytes, wrote %d.\n", msg_len + KEY_PADDING , ctrl);
							free(sym_key_of_peer);
							close(socket);
							return;
						}

						memset(output_from_command, '\0', sizeof output_from_command);

						memcpy(output_from_command, &output_buffer[count], strlen(&output_buffer[count]));

						stored = strlen(&output_buffer[count]);

					} else {

						memcpy(output_from_command + stored, output_buffer, output_len);
						stored += output_len;
					
					}
				
				}

				status = pclose(process);

				if ( status ) {
					// Error...
					memset(output_from_command, '\0', sizeof output_from_command);
					sprintf(output_from_command, "Error: That command did not work to well.\n");
				} 

				msg_len = (uint64_t) strlen(output_from_command), sz_send = msg_len;

				if ( is_little_endian ) {
					uint64_t tmp;
					swap(&sz_send, &tmp, 8);
					sz_send = tmp;
				}

				memcpy(server_talk, &sz_send, KEY_PADDING);
				sprintf(server_talk + KEY_PADDING, "%s", output_from_command);

				symmetric_key_crypto(symmetric_key_here, sizeof symmetric_key_here, server_talk, msg_len + KEY_PADDING, ENCRYPT);

				ctrl = write(socket, server_talk, msg_len + KEY_PADDING); // writing the encrypted version of the greeting

				if ( ctrl != msg_len + KEY_PADDING ) {
					log_error("Attempted to write %" PRIu64 " bytes, wrote %d.\n", msg_len + KEY_PADDING , ctrl);
					free(sym_key_of_peer);
					close(socket);
					return;
				}
			}
		}

	}

end:

	close(socket);
	free(sym_key_of_peer); // Used to decode the messages sent from the peer during this session, it will no longer be needed leaving this function.
	log("Sec session closed!\n");

}

void sec_session_client(int socket)
{
	unsigned char symmetric_key_here[STNDRD_SYM_KEY_LEN], *key_len_decode_buffer,
			*sym_key_over_tcp, *sym_key_of_peer, key_len_over_tcp[8], *peer_key_len_over_tcp,
			 	cmd, read_write_buffer[MAX_DATA_LEN];
	char server_talk[MAX_DATA_LEN + 1], input_buffer[MAX_DATA_LEN + 1];
	rsa_session_t private_session, public_session; // in the public only n and e are known.
	uint64_t n_over_tcp, e_over_tcp, key_len, peer_sym_key_len,
				sz_send, msg_len;
	int ctrl;

	bool is_little_endian = false;

	{
		int n = 1;
		is_little_endian = (*((char*)&n) == 1);
	}

	initialize_session(&private_session, symmetric_key_here, sizeof symmetric_key_here); // this process might take some time (not to long though)
	
	if ( is_little_endian == true ) {
		// Converting the newly received coefficients to little endian.

		swap(&private_session.n, &n_over_tcp, sizeof private_session.n );
		swap(&private_session.e ,&e_over_tcp, sizeof private_session.e );

	} else {

		n_over_tcp = private_session.n;
		e_over_tcp = private_session.e;

	}

	// Writing the command of sec-server initialization
	cmd = SEC_SESSION;

	memset(read_write_buffer, '\0', sizeof read_write_buffer);
	
	memcpy(read_write_buffer, &cmd, 1);
	memcpy(read_write_buffer + 1, &n_over_tcp, KEY_PADDING);
	memcpy(read_write_buffer + KEY_PADDING + 1, &e_over_tcp, KEY_PADDING);

	// writing the command as well as the RSA public keys of the private session
	printf("Writing the public RSA keys to the server...\n");
	ctrl = write(socket, read_write_buffer, KEY_PADDING*2 + 1);

	if ( ctrl != 2 * KEY_PADDING + 1 ) {
		log_error("Attempted to write %d bytes, wrote %d.\n", 2 * KEY_PADDING + 1, ctrl);
		close(socket);
		return;
	}

	// Getting the RSA public keys from the server
	memset(read_write_buffer, '\0', sizeof read_write_buffer);
	ctrl = read(socket, read_write_buffer, KEY_PADDING * 2);

	if ( ctrl != 2 * KEY_PADDING ) {
		log_error("Attempted to read %d bytes, got %d.\n", 2 * KEY_PADDING, ctrl);
		close(socket);
		return;
	}

	memcpy(&n_over_tcp, read_write_buffer, KEY_PADDING);
	memcpy(&e_over_tcp, read_write_buffer + KEY_PADDING, KEY_PADDING);

	if ( is_little_endian == true ) {
		// making all the RSA coefficients to be stored in little endian order

		swap(&n_over_tcp, &public_session.n, sizeof public_session.n );
		swap(&e_over_tcp, &public_session.e, sizeof public_session.e );

	} else {

		public_session.n = n_over_tcp;
		public_session.e = e_over_tcp;
		
	}

	// Reading the encrypted value of the length of the symmetric key
	printf("Reading the public RSA keys from the server...\n");
	ctrl = read(socket, key_len_over_tcp, KEY_PADDING );

	if ( ctrl != KEY_PADDING ) {
		log_error("Attempted to read %d bytes, got %d.\n", KEY_PADDING, ctrl);
		close(socket);
		return;
	}

	// Decoding the key length
	key_len_decode_buffer = rsa_decode(&private_session, key_len_over_tcp, KEY_PADDING ); // might take some time

	key_len = 0, key_len += key_len_decode_buffer[0]; // the decoded key length is only one char long
	free(key_len_decode_buffer); // its also heap allocated (consequence of current 'rsa_decode' implementation)

	if ( key_len > MAX_SYM_KEY_LEN ) {
		printf("Read key length: %" PRIu64 " too large. Session interrupted.\n", key_len);
		close(socket);
		return; 
	}

	sym_key_over_tcp = (unsigned char*) malloc(key_len); // allocating space for the encrypted symemtric key to be read

	peer_sym_key_len = key_len / KEY_PADDING;
	printf("Reading the encrypted symmetric key of the server...\n");
	ctrl = read(socket, sym_key_over_tcp, key_len); // reading the encrypted symmetric key used by the server

	if ( ctrl != key_len ) {
		log_error("Attempted to read %" PRIu64 " bytes, got %d.\n", key_len, ctrl);
		close(socket);
		return;
	}

	// Decoding the symmetric key of the client
	sym_key_of_peer = rsa_decode(&private_session, sym_key_over_tcp, key_len); // will take some time..
	free(sym_key_over_tcp); // freeing this now

	key_len = KEY_PADDING * sizeof(symmetric_key_here);

	peer_key_len_over_tcp = rsa_encode(&public_session, is_little_endian ? &((unsigned char*)&key_len)[0] : &((unsigned char*)&key_len)[KEY_PADDING-1], 1);

	// writing the encoded value of the expected length of the symmetric key
	printf("Sending the encrypted symmetric key used by us to the server...\n");

	ctrl = write(socket, peer_key_len_over_tcp, KEY_PADDING);
	free(peer_key_len_over_tcp);

	if ( ctrl != KEY_PADDING ) {
		log_error("Attempted to write %d bytes, wrote %d.\n", KEY_PADDING, ctrl);
		free(sym_key_of_peer);
		close(socket);
		return;
	}

	// writing the encoded version of the symmetric key
	sym_key_over_tcp = rsa_encode(&public_session, symmetric_key_here , sizeof symmetric_key_here );
	
	ctrl = write(socket, sym_key_over_tcp, sizeof(symmetric_key_here) * KEY_PADDING );
	free(sym_key_over_tcp);

	if ( ctrl != sizeof(symmetric_key_here) * KEY_PADDING ) {
		log_error("Attempted to write %u bytes, got %d.\n", sizeof(symmetric_key_here) * KEY_PADDING , ctrl);
		free(sym_key_of_peer);
		close(socket);
		return;
	}


// ================================ Connection is established! All parameters have been configured. ================================ //
	printf(" ================= Connection established! =================\n");

	while ( 1 ) {

		memset(server_talk, '\0', sizeof server_talk);
		memset(input_buffer, '\0', sizeof input_buffer);

		ctrl = read(socket, server_talk, KEY_PADDING); // Get the encrypted length of the message to be passed..

		if ( ctrl != KEY_PADDING ) {
			log_error("Attempted to read %d bytes, got %d.\n", KEY_PADDING, ctrl);
			free(sym_key_of_peer);
			close(socket);
			return;
		}

		symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, 8, DECRYPT);

		if ( is_little_endian ){
			uint64_t tmp = 0;
			swap(server_talk, &tmp, 8);
			sz_send = tmp;
		} else {
			uint64_t tmp = 0;
			memcpy(&tmp, server_talk, 8);
			sz_send = tmp;
		}

		if ( sz_send > MAX_DATA_LEN ){
			free(sym_key_of_peer);
			close(socket);
			printf("[%s] ERROR AT: %d\n", __func__, __LINE__);
			return;
		}

		symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, 8, ENCRYPT);

		ctrl = read(socket, server_talk + 8, sz_send);

		if ( ctrl != sz_send ) {
			log_error("Attempted to read %" PRIu64 " bytes, got %d.\n", sz_send, ctrl);
			free(sym_key_of_peer);
			close(socket);
			return;
		}

		symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, sz_send + 8, DECRYPT);

		printf("%s", server_talk + KEY_PADDING);

		while ( sz_send == MAX_DATA_LEN ) {

			memset(server_talk, '\0', sizeof server_talk);

			ctrl = read(socket, server_talk, KEY_PADDING); // Get the encrypted length of the message to be passed..

			if ( ctrl != KEY_PADDING ) {
				log_error("Attempted to read %d bytes, got %d.\n", KEY_PADDING, ctrl);
				free(sym_key_of_peer);
				close(socket);
				return;
			}

			symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, KEY_PADDING, DECRYPT);

			if ( is_little_endian ){
				uint64_t tmp = 0;
				swap(server_talk, &tmp, 8);
				sz_send = tmp;
			} else {
				uint64_t tmp = 0;
				memcpy(&tmp, server_talk, 8);
				sz_send = tmp;
			}

			if ( sz_send > MAX_DATA_LEN ){
				free(sym_key_of_peer);
				close(socket);
				printf("[%s] ERROR AT: %d\n", __func__, __LINE__);
				return;
			}

			symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, KEY_PADDING, ENCRYPT);

			ctrl = read(socket, server_talk + KEY_PADDING, sz_send);

			if ( ctrl != sz_send ) {
				log_error("Attempted to read %" PRIu64 " bytes, got %d.\n", sz_send, ctrl);
				free(sym_key_of_peer);
				close(socket);
				return;
			}

			symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, sz_send + KEY_PADDING, DECRYPT);

			printf("%s", server_talk + KEY_PADDING);

		}

		printf(">>");

		// Read user input
		fgets(input_buffer, MAX_DATA_LEN, stdin);

		if ( strstr(input_buffer, "quit") != NULL ) {
			printf("Connection closed!\n");
			goto end;
		} else {
						// Start by outputting the greeting
			msg_len = (uint64_t) strlen(input_buffer), sz_send = msg_len;

			if ( is_little_endian ) {
				uint64_t tmp;
				swap(&sz_send, &tmp, 8);
				sz_send = tmp;
			}

			memcpy(server_talk, &sz_send, KEY_PADDING);
			memcpy(server_talk + KEY_PADDING, input_buffer, msg_len);

			symmetric_key_crypto(symmetric_key_here, sizeof symmetric_key_here, server_talk, msg_len + 8, ENCRYPT);

			ctrl = write(socket, server_talk, msg_len + KEY_PADDING); // writing the encrypted version of the greeting

			if ( ctrl != msg_len + KEY_PADDING ) {
				log_error("Attempted to write %llu bytes, wrote %d.\n", msg_len + KEY_PADDING , ctrl);
				printf("Unexpected error: Failed to output the data to the server. Server not responding.\n");
				goto end;
			}

		}

	}

end:
	close(socket);
	free(sym_key_of_peer); // Used to decode the messages sent from the peer during this session, it will no longer be needed leaving this function.
}


