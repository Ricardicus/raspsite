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
				server_talk[MAX_DATA_LEN + 1], read_write_buffer[MAX_DATA_LEN];
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
	memcpy(read_write_buffer, &n_over_tcp, 8);
	memcpy(read_write_buffer + KEY_PADDING, &e_over_tcp, 8);

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

	memcpy(server_talk, &sz_send, KEY_PADDING);
	memcpy(server_talk + KEY_PADDING, server_responses[GREET], msg_len);

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

	close(socket);
	free(sym_key_of_peer); // Used to decode the messages sent from the peer during this session, it will no longer be needed leaving this function.
}

void sec_session_client(int socket)
{
	unsigned char symmetric_key_here[STNDRD_SYM_KEY_LEN], *key_len_decode_buffer,
			*sym_key_over_tcp, *sym_key_of_peer, key_len_over_tcp[8], *peer_key_len_over_tcp,
				server_talk[MAX_DATA_LEN + 1], cmd, read_write_buffer[MAX_DATA_LEN];
	rsa_session_t private_session, public_session; // in the public only n and e are known.
	uint64_t n_over_tcp, e_over_tcp, key_len, peer_sym_key_len,
				sz_send;
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
	ctrl = read(socket, sym_key_over_tcp, key_len); // reading the encrypted symmetric key used by the server

	if ( ctrl != key_len ) {
		log_error("Attempted to read %llu bytes, got %d.\n", key_len, ctrl);
		close(socket);
		return;
	}

	// Decoding the symmetric key of the client
	sym_key_of_peer = rsa_decode(&private_session, sym_key_over_tcp, key_len); // will take some time..
	free(sym_key_over_tcp); // freeing this now

	key_len = KEY_PADDING * sizeof(symmetric_key_here);

	peer_key_len_over_tcp = rsa_encode(&public_session, is_little_endian ? &((unsigned char*)&key_len)[0] : &((unsigned char*)&key_len)[KEY_PADDING-1], 1);

	// writing the encoded value of the expected length of the symmetric key
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
		log_error("Attempted to write %lu bytes, got %d.\n", sizeof(symmetric_key_here) * KEY_PADDING , ctrl);
		free(sym_key_of_peer);
		close(socket);
		return;
	}


// ================================ Connection is established! All parameters have been configured. ================================ //
	printf("Connection is established!\n");

	memset(server_talk, '\0', sizeof server_talk);
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

	if ( sz_send > 255 ){
		free(sym_key_of_peer);
		close(socket);
		printf("[%s] ERROR AT: %d\n", __func__, __LINE__);
		return;
	}

	symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, 8, ENCRYPT);

	ctrl = read(socket, server_talk + 8, sz_send);

	if ( ctrl != sz_send ) {
		log_error("Attempted to read %llu bytes, got %d.\n", sz_send, ctrl);
		free(sym_key_of_peer);
		close(socket);
		return;
	}

	symmetric_key_crypto(sym_key_of_peer, peer_sym_key_len, server_talk, sz_send + 8, DECRYPT);

	printf("CLIENT RECEIEVED THE FOLLOWING MESSAGE FROM THE SERVER:\n");
	printf("%s\n", server_talk + 8);

	close(socket);
	free(sym_key_of_peer); // Used to decode the messages sent from the peer during this session, it will no longer be needed leaving this function.
}


