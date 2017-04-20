#ifdef __cplusplus
extern "C" {
#endif 

#ifndef PASSWORD_H
#define PASSWORD_H

/* 
* The password required to enter the restricted pages,
* and the encrypted shell session. The password 
* may be changed to something else than what is displayed here.. 
* You can also add more passwords, but make sure that you also
* enter more 'strcmp' statements where necessary in 'http.c'.
*/
#define STANDARD_USER_PASSWORD "admin:password"

#endif 

#ifdef __cplusplus
}
#endif 