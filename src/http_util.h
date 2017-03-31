#ifdef __cplusplus
extern "C" {
#endif 

#ifndef HTTP_UTIL_H
#define HTTP_UTIL_H

#include <ctype.h>

/*
* Wrapper for base 64 decoding and encoding found online at:
* stackoverflow.com: http://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
*/

char *base64_encode(const unsigned char *, size_t, size_t *);
unsigned char *base64_decode(const char *,size_t, size_t *);
void build_decoding_table(void);
void base64_cleanup(void); 

/*
* Function for url decoding found online at:
* stackoverflow.com: http://stackoverflow.com/questions/2673207/c-c-url-decode-library
*/
void urldecode2(char *, const char *);

#endif

#ifdef __cplusplus
}
#endif 