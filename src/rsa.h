#ifdef __cplusplus
extern "C" {
#endif

#ifndef RSA_H
#define RSA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define KEY_PADDING			8

typedef struct prime_pair_t {
	uint64_t p;
	uint64_t q;
} prime_pair_t;

/* Using a keysize of 64 bits .. */
typedef struct rsa_session_t {
	uint64_t d;
	uint64_t e;
	uint64_t n;
	uint64_t lambda;
} rsa_session_t;

void 
generate_rsa_keys(rsa_session_t*);

char *
rsa_encode(rsa_session_t*, void *, size_t);

char *
rsa_decode(rsa_session_t*, void *, size_t);

void 
symetric_key_crypto(void*, size_t, void *, size_t, bool);

void 
generate_symmetric_key(void *, size_t);


#endif

#ifdef __cplusplus
}
#endif