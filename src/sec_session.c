#include "sec_session.h"

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
*			 4		 MSG LEN	
*
* All messages are encrypted with its respective decrypted symmetric key 	
* using the function 'symetric_key_crypto' in "rsa.c".
*
* =======================================================
*/
void sec_session(int socket)
{
	char cmd, symmetric_key_here[20], *key_len_decode_buffer,
			*sym_key_over_tcp, *sym_key_of_peer, *key_len_over_tcp;
	rsa_session_t private_session, public_session; // in the public only n and e are known.
	uint64_t n_over_tcp, e_over_tcp, key_len;

	bool is_little_endian = false;

	{
		int n = 1;
		is_little_endian = (*((char*)&n) == 1);
	}

	initialize_session(&private_session, symmetric_key_here, sizeof symmetric_key_here); // this process might take some time (not to long though)


	if ( is_little_endian == true ) {
		// making all the RSA coefficients appear over TCP in big endian order

		swap(&private_session.n, &n_over_tcp, sizeof private_session.n );
		swap(&private_session.e, &e_over_tcp, sizeof private_session.e );

	}

	// Sending the RSA public keys of the private session
	write(socket, &n_over_tcp, 8), write(socket, &e_over_tcp, 8);

	// Getting the RSA public keys of the public session
	read(socket, &n_over_tcp, 8), read(socket, &e_over_tcp, 8);

	// Reading the encrypted value of the length of the symmetric key
	read(socket, &key_len_over_tcp, 8);

	// Decoding the key length
	key_len_decode_buffer = rsa_decode(&private_session, &key_len_over_tcp, sizeof key_len_over_tcp ); // might take some time

	if ( is_little_endian == true ) {
		// making all the values tranfered over TCP appear in little endian order

		swap(&n_over_tcp, &public_session.n, sizeof public_session.n );
		swap(&e_over_tcp, &public_session.e, sizeof public_session.e );

	} 

	key_len = (uint64_t) key_len_decode_buffer[0]; // the decoded key length is only one char long
	free(key_len_decode_buffer); // its also heap allocated (consequence of current 'rsa_decode' implementation)

	if ( key_len > MAX_SYM_KEY_LEN ) {
		printf("Read key length: %" PRIu64 " too large. Session interrupted.\n", key_len);
		write(socket, "error", 5);
		close(socket);
		return; 
	}

	sym_key_over_tcp = (char*) malloc(key_len); // allocating space for the encrypted symemtric key to be read

	read(socket, sym_key_over_tcp, key_len); // reading the encrypted symmetric key used by the other peer

	sym_key_of_peer = rsa_decode(&public_session, sym_key_over_tcp, key_len); // will take some time..
	free(sym_key_over_tcp); // freeing this now

	printf("Read decoded symmetric key of peer: %s\n", sym_key_of_peer);

	key_len = KEY_PADDING * sizeof symmetric_key_here;
	key_len_over_tcp = rsa_encode(&private_session, is_little_endian ? &key_len[0] : &key_len[KEY_PADDING-1], 1);

	// writing the encoded value of the expected length of the symmetric key
	write(socket, key_len_over_tcp, 8);
	free(key_len_over_tcp);

	// writing the encoded version of the symmetric key
	sym_key_over_tcp = rsa_encode(&private_session, symmetric_key_here , sizeof symmetric_key_here );
	write(socket, sym_key_over_tcp, key_len);
	free(sym_key_over_tcp);

	/*
	* All parameters are transferred and the secure connection is established.
	* ........... 				More to come next! 			................
	*/


	free(sym_key_of_peer); // Used to decode the messages sent from the peer during this session, it will no longer be needed leaving this function.
}