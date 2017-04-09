#include "rsa.h"

static prime_pair_t * generate_prime_pair(const char * path){
	FILE *fp, *pr;
	char number_buffer[20], cmd[100];
	uint64_t limit, index_one, index_two, i;

	time_t t;

	srand((unsigned) time(&t));

	prime_pair_t * pair;

	fp = fopen(path, "r");

	if ( fp == NULL ){
		return NULL;
	}

	memset(cmd, '\0', sizeof(cmd));
	snprintf(cmd, sizeof(cmd)-1, "cat %s |wc -l |awk '{ print $1; }'", path);

	pr = popen ( cmd , "r"); 

	while ( fgets(number_buffer, sizeof(number_buffer) - 1, pr) != NULL ) {
		limit = (uint64_t) atol(number_buffer);
	}

	pclose(pr);

	pair = malloc(sizeof(prime_pair_t));

	index_one = rand() % limit, index_two = rand() % limit, i=0;

	if ( index_one == index_two ) // special case..
		index_one = index_one == 0 ? 1 : --index_one;

	while ( fgets(number_buffer, sizeof(number_buffer) - 1, fp) != NULL ) {
		if ( i == index_one ) {
			pair->p = (uint64_t) atol(number_buffer);
		} else if ( i == index_two ){
			pair->q = (uint64_t) atol(number_buffer);
		}
		memset(number_buffer, '\0', sizeof number_buffer);
		++i;
	}

	fclose(fp);
	return pair;
}

static uint64_t inverse_euclid_extended(uint64_t a, uint64_t p) {
    uint64_t new = 1, old = 0, q = p, r, h;
    int pos = 0;
    while (a > 0) {
        r = q%a;
        q = q/a;
        h = q*new + old;
        old = new;
        new = h;
        q = a;
        a = r;
        pos = !pos;
    }
    return pos ? old : (p - old);
}

static int is_even(uint64_t a) {
	return (a & 1) == 0;
}

static uint64_t gcd(uint64_t a, uint64_t b) {
	//Step 5 (base case)
	if (a == b)
		return a;
	//Step 1.
	//Check if any is zero.
	if (a == 0 && b != 0)
		return b;
	else if (a != 0 && b == 0)
		return a;
	else if (a == 0 && b == 0)
		return 0;
	//Step 2.
	//Check if both are even.
	if (is_even(a) && is_even(b))
		return gcd(a >> 1, b >> 1) << 1;
	//Step 3.
	//Handle if one is odd.
	if (is_even(a))
		return gcd(a >> 1, b);
	else if (is_even(b))
		return gcd(b >> 1, a);
	//Step 4.
	//Both are odd.
	if (a >= b)
		return gcd((a-b) >> 1, b);
	else if (a < b)
		return gcd((b-a) >> 1, a);

	return 0;
}

static uint64_t lcm(uint64_t p, uint64_t q){
	return (p / gcd(p,q)) * q ;
}

/*
* n will be an uint64_t (2^32 - 1), p and q must be less than sqrt of that...
* -> Generate primes from 3 to (2^16 - 1), i have in "etc/primes/primes.txt" the primes from 2 to 65521. 
*/
void 
generate_rsa_keys(rsa_session_t* session){
	uint64_t prime_p, prime_q, d; // these will be randomly selected somehow...
	uint64_t lambda, i = 2, e = 0, limit, c = 0; // e is public, d is kept private
	uint64_t n, tmp1, tmp2;

	prime_pair_t * pair;

	/*
	* Generate the primes .....
	*/

	pair = generate_prime_pair("etc/primes/primes.txt"); // reads from the primes..
	prime_q = pair->q, prime_p = pair->p; 
	free(pair);

	n = prime_p*prime_q;

	tmp1 = tmp2 = 0;
	tmp1 = prime_p, tmp2 = prime_q;
	lambda = lcm(tmp1-1, tmp2-1);

	session->lambda = lambda;

	limit = (1 + rand() % 8);

	for (; i < lambda; ++i) {
		if ( gcd(lambda, i) == 1 ){
			e = i;			
			if ( ++c >= limit ) {
				break;
			} 
		}
	}

	d = inverse_euclid_extended(e, lambda);

	session->n = n;
	session->e = e;
	session->d = d;

}

/*
* Uses the coefficients in the session to encode
* the content in the msg. Returns a byte string that must 
* be freed by the user. It contains the data padded in big-endian order.
* the size of the output data will be 8 times the input data size
* (exluding the null terminating byte).
*
* 	params: 
*		@session - coefficients for encryption (n and e set, rest not used)
*		@msg - Pointer to the first occurence of the data to be encoded
*		@sz - the number of bytes to be encoded
*	
*	returns:
*		on succes - null terminated string of the data that is encoded
*		on failure- NULL
*/
char *
rsa_encode(rsa_session_t* session, void * msg, size_t sz) {

	uint64_t n, e,data_enc_lu, c=0;
	char *output, data_raw;
	size_t output_sz,i=0;
	int indx;

	bool is_little_endian = false;

	{
		int n = 1;
		is_little_endian = (*((char*)&n) == 1);
	}

	n = session->n;
	e = session->e;

	output_sz = sz*KEY_PADDING+1;

	output = malloc(output_sz);

	if ( output == NULL ){
		fprintf(stderr, "Malloc failed\n");
		return NULL;
	}

	memset(output, '\0', output_sz);

	for (; i<sz ; ++i) {
		
		data_raw = ((char*)msg)[i];
		data_enc_lu = 0, data_enc_lu = data_raw;

		c = 2;
		uint64_t k = data_enc_lu;
		while ( c <= e ) {
			data_enc_lu = (data_enc_lu * k) % n;
			++c;
		}

		printf("data_enc_lu (%zu): [%llu]\n", i, data_enc_lu);

		indx = 0;
		for (; indx < KEY_PADDING; ++indx ){	
			output[KEY_PADDING*i+indx] = is_little_endian ? ((char*)&data_enc_lu)[KEY_PADDING-1-indx] : ((char*)&data_enc_lu)[indx];
		}

	}

	return output;

}

/*
* Uses the coefficients in the session to decode
* the content in the msg. This is a slow decryptin since the coefficients
* for decoding usually end up being pretty big. 
* It should only be used to decrypt symmetric keys used in the rest of the session.
*
* The function returns a byte string that must be freed by the user. 
* It contains the data in its original order.
* the size of the output data will be KEY_PADDING times smaller than the input datas size
* (exluding the null terminating byte).
*
* 	params: 
*		@session - coefficients for decryption (n and d set, rest not used)
*		@msg - Pointer to the first occurence of the data to be decoded
*			   It must be represented with a size of a multiple of KEY_PADDING, and it must 
*			   also be in big-endian format (as output is from the rsa_encrypt method above..).
*		@sz - the number of bytes to be decoded
*	
*	returns:
*		on succes - null terminated string of the data that is decoded
*		on failure- NULL
*/
char *
rsa_decode(rsa_session_t* session, void * msg, size_t sz)
{
	uint64_t n, d,data_dec_lu, data_raw, c=0;
	char *output;
	size_t output_sz,i=0;
	int q;

	if ( sz % KEY_PADDING != 0) {
		fprintf(stderr,"[%s] size of data must be a multiple of %d!\n", __func__, KEY_PADDING);
		return NULL;
	} 

	bool is_little_endian = false;

	{
		int n = 1;
		is_little_endian = (*((char*)&n) == 1);
	}

	n = session->n;
	d = session->d;

	output_sz = sz/KEY_PADDING+1;

	output = malloc(output_sz);
	printf("[%s] malloc %zu bytes...\n",__func__, output_sz);
	if ( output == NULL ){
		fprintf(stderr, "Malloc failed\n");
		return NULL;
	}

	memset(output, '\0', output_sz);

	for (; i<sz ; i=i+KEY_PADDING) {

		q = 0;
		for (; q<KEY_PADDING; ++q){
			((char*)&data_raw)[q] = is_little_endian ? ((char*)msg)[i + KEY_PADDING-1-q] : ((char*)msg)[i + q];
		}

		data_dec_lu = data_raw;

		printf("data_dec_lu (%zu): [%llu]\n", i/KEY_PADDING, data_dec_lu);

		c = 2;
		uint64_t k = data_dec_lu;

		while ( c <= d ) {
			data_dec_lu = (data_dec_lu * k) % n;
			++c;
		}

		output[i/KEY_PADDING] = is_little_endian ? ((char*)&data_dec_lu)[0] : ((char*)&data_dec_lu)[KEY_PADDING-1];

	}

	return output;
}

/*
* Performs a symmetric key based encryption algorithm developed to be used for this server program.
* Important: In order to prevent segfault make sure that at least 'data_size' number of bytes
* are allocated under the pointer to 'data'. This function works on the assumption that this is true.
*
* params:
*	@key - a set of char values, 'key_size' of them, representing the key
*	@key_size - the number of chars in the key.
*	@data - data to be encrypted.
*	@data_size - the number of bytes in the data that is for be decrypted
*	@encrypt - If true then encryption should be performed else decryption is performed.
*/
void 
symetric_key_crypto(void* key, size_t key_size, void * data, size_t data_size, bool encrypt)
{
	char prev_riddler[2], riddler; 
	size_t i = 0;
	int pw, c, toggle;

	pw = 0;
	i = 0;
	toggle = 1; 

	prev_riddler[0] = 0;
	prev_riddler[1] = 0;

	while ( i < data_size ) {

		riddler = ((char*)key)[i % key_size];

		int pw_c = pw;
		while( pw_c > 0 ){
			riddler *= riddler;
			--pw_c;
		}

		if ( i % 60 == 0 )
			toggle = (pw % 120) > 60 ? -1 : 1;

		if ( i % key_size == 0 && i > 0 )
			pw += toggle;

		// the old value
		c = ((char* )data)[i]; 
		// setting the new value
		((char*) data)[i] = ( encrypt == true ? c + riddler + prev_riddler[0] + prev_riddler[1]
			: c - riddler - prev_riddler[0] - prev_riddler[1]);
	
		prev_riddler[0] = prev_riddler[1];
		prev_riddler[1] = riddler;

		++i;
	}
}


void 
generate_symmetric_key(void * buffer, size_t sz){
	time_t t;
	size_t i = 0;

	srand((unsigned) time(&t));

	while ( i < sz ) {

		((char*)buffer)[i] = rand() % 256; // Random values ...

		++i;
	}

}
