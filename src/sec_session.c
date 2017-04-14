#include "sec_session.h"

static char *server_responses[] = (char *[]) { 
	"Welcome!\nFor help enter 'help'.", // Greeting
	"These are your alternatives: \n* 'help': display this tutorial.\n\
	* 'shell': enter the shell over this 'secure' channel.\n \
	* 'quit': exit this session.\n-- More to come (possibly) --",
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
*			8		8		8					KEY LEN (1-255) * 8
* 3. 	Client sends its symmetric key encrypted using the servers 
*		rsa coefficients:
*		| KEY LEN (encrypted)| KEY (encrypted) |
* Bytes:
*			8		KEY LEN (1-255)
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
void sec_session_server(int socket)
{
	unsigned char symmetric_key_here[STNDRD_SYM_KEY_LEN], *key_len_decode_buffer,
			*sym_key_over_tcp, *sym_key_of_peer, *key_len_over_tcp, peer_key_len_over_tcp[8],
				server_talk[MAX_DATA_LEN + 1];
	rsa_session_t private_session, public_session; // in the public only n and e are known.
	uint64_t n_over_tcp, e_over_tcp, key_len, peer_sym_key_len,
				sz_send, msg_len;

	bool is_little_endian = false;

	{
		int n = 1;
		is_little_endian = (*((char*)&n) == 1);
	}

	initialize_session(&private_session, symmetric_key_here, sizeof symmetric_key_here); // this process might take some time (not to long though)

	// Getting the RSA public keys of the public session
	read(socket, &n_over_tcp, 8), read(socket, &e_over_tcp, 8);

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
	write(socket, &n_over_tcp, 8), write(socket, &e_over_tcp, 8);

	key_len = KEY_PADDING * sizeof(symmetric_key_here);

	key_len_over_tcp = rsa_encode(&public_session, is_little_endian ? &((unsigned char*)&key_len)[0] : &((unsigned char*)&key_len)[KEY_PADDING-1], 1);

	// writing the encoded value of the expected length of the symmetric key
	write(socket, key_len_over_tcp, 8);
	free(key_len_over_tcp);

	// writing the encoded version of the symmetric key
	sym_key_over_tcp = rsa_encode(&public_session, symmetric_key_here , sizeof symmetric_key_here );
	write(socket, sym_key_over_tcp, key_len);
	free(sym_key_over_tcp);

	// Reading the encrypted value of the length of the symmetric key
	read(socket, peer_key_len_over_tcp, 8);

	// Decoding the key length
	key_len_decode_buffer = rsa_decode(&private_session, peer_key_len_over_tcp, sizeof peer_key_len_over_tcp ); // might take some time

	key_len = 0, key_len += key_len_decode_buffer[0]; // the decoded key length is only one char long
	free(key_len_decode_buffer); // its also heap allocated (consequence of current 'rsa_decode' implementation)

	if ( key_len > MAX_SYM_KEY_LEN ) {
		printf("Read key length: %" PRIu64 " too large. Session interrupted.\n", key_len);
		write(socket, "error", 5);
		close(socket);
		return; 
	}

	sym_key_over_tcp = (unsigned char*) malloc(key_len); // allocating space for the encrypted symemtric key to be read

	peer_sym_key_len = key_len / KEY_PADDING;
	read(socket, sym_key_over_tcp, key_len); // reading the encrypted symmetric key used by the other peer

	// Decoding the symmetric key of the client
	sym_key_of_peer = rsa_decode(&private_session, sym_key_over_tcp, key_len); // will take some time..
	free(sym_key_over_tcp); // freeing this now

	/*
	* All parameters are transferred and the secure connection is established.
	* ........... 				More to come next! 			................
	*/

	printf("Connection initalized by client is established!\n");

	// Start by outputting the greeting
	msg_len = (uint64_t) strlen(server_responses[GREET]), sz_send = msg_len;

	if ( is_little_endian ) {
		uint64_t tmp;
		swap(&sz_send, &tmp, 8);
		sz_send = tmp;
	}

	memcpy(server_talk, &sz_send, 8);
	memcpy(server_talk + 8, server_responses[GREET], msg_len);

	symmetric_key_crypto(symmetric_key_here, sizeof symmetric_key_here, server_talk, msg_len + 8, ENCRYPT);

	write(socket, server_talk, msg_len + 8); // writing the encrypted version of the greeting

	/*
	* More to come from here on out! :) 
	* We have established the secure communication channel!
	*/

	close(socket);
	free(sym_key_of_peer); // Used to decode the messages sent from the peer during this session, it will no longer be needed leaving this function.
}

void sec_session_client(int socket)
{
	unsigned char symmetric_key_here[STNDRD_SYM_KEY_LEN], *key_len_decode_buffer,
			*sym_key_over_tcp, *sym_key_of_peer, key_len_over_tcp[8], *peer_key_len_over_tcp,
				server_talk[MAX_DATA_LEN + 1], cmd;
	rsa_session_t private_session, public_session; // in the public only n and e are known.
	uint64_t n_over_tcp, e_over_tcp, key_len, peer_sym_key_len,
				sz_send;

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
	write(socket, &cmd, 1);

	// writing the RSA public keys of the private session
	write(socket, &n_over_tcp, 8), write(socket, &e_over_tcp, 8);

	// Getting the RSA public keys from the server
	read(socket, &n_over_tcp, 8), read(socket, &e_over_tcp, 8);

	if ( is_little_endian == true ) {
		// making all the RSA coefficients to be stored in little endian order

		swap(&n_over_tcp, &public_session.n, sizeof public_session.n );
		swap(&e_over_tcp, &public_session.e, sizeof public_session.e );

	} else {

		public_session.n = n_over_tcp;
		public_session.e = e_over_tcp;
		
	}

	// Reading the encrypted value of the length of the symmetric key
	read(socket, key_len_over_tcp, KEY_PADDING );

	// Decoding the key length
	key_len_decode_buffer = rsa_decode(&private_session, key_len_over_tcp, KEY_PADDING ); // might take some time

	key_len = 0, key_len += key_len_decode_buffer[0]; // the decoded key length is only one char long
	free(key_len_decode_buffer); // its also heap allocated (consequence of current 'rsa_decode' implementation)

	if ( key_len > MAX_SYM_KEY_LEN ) {
		printf("Read key length: %" PRIu64 " too large. Session interrupted.\n", key_len);
		write(socket, "error", 5);
		close(socket);
		return; 
	}

	sym_key_over_tcp = (unsigned char*) malloc(key_len); // allocating space for the encrypted symemtric key to be read

	peer_sym_key_len = key_len / KEY_PADDING;
	read(socket, sym_key_over_tcp, key_len); // reading the encrypted symmetric key used by the server

	// Decoding the symmetric key of the client
	sym_key_of_peer = rsa_decode(&private_session, sym_key_over_tcp, key_len); // will take some time..
	free(sym_key_over_tcp); // freeing this now

	key_len = KEY_PADDING * sizeof(symmetric_key_here);

	peer_key_len_over_tcp = rsa_encode(&public_session, is_little_endian ? &((unsigned char*)&key_len)[0] : &((unsigned char*)&key_len)[KEY_PADDING-1], 1);

	// writing the encoded value of the expected length of the symmetric key
	write(socket, peer_key_len_over_tcp, 8);
	free(peer_key_len_over_tcp);

	// writing the encoded version of the symmetric key
	sym_key_over_tcp = rsa_encode(&public_session, symmetric_key_here , sizeof symmetric_key_here );
	write(socket, sym_key_over_tcp, sizeof(symmetric_key_here) * KEY_PADDING );
	free(sym_key_over_tcp);

// ================================ Connection is established! All parameters have been configured. ================================ //
	printf("Connection is established!\n");

	memset(server_talk, '\0', sizeof server_talk);
	read(socket, server_talk, 8); // Get the encrypted length of the message to be passed..

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

	if ( sz_send > 255 ){
		free(sym_key_of_peer);
		close(socket);
		printf("[%s] ERROR AT: %d\n", __func__, __LINE__);
		return;
	}

	symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, 8, ENCRYPT);

	read(socket, server_talk + 8, sz_send);

	symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, sz_send + 8, DECRYPT);

	printf("CLIENT RECEIEVED THE FOLLOWING MESSAGE FROM THE SERVER:\n");
	printf("%s\n", server_talk + 8);

	close(socket);
	free(sym_key_of_peer); // Used to decode the messages sent from the peer during this session, it will no longer be needed leaving this function.
}


