#include "sec_session.h"

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
*		| RSA N | RSA E | KEY LEN | KEY (encrypted) |
* Bytes:
*			8		8		1		KEY LEN (1-255)
* 3. 	Client sends its symmetric key encrypted using the servers 
*		rsa coefficients:
*		| KEY LEN | KEY (encrypted) |
* Bytes:
*			1		KEY LEN (1-255)
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
	char cmd,symmetric_key[100];
	rsa_session_t session;

	initialize_session(&session, symmetric_key, sizeof symmetric_key);

/*
*	coming soon... 
*/ 


}