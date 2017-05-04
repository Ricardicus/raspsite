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
* Choose this password with care!
*/
#define STANDARD_USER_PASSWORD "admin:password"

/* 
* Login and other information for sending mails from the raspberry pi
* must be set to a valid gmail account in order for the mail feature
* to function. I have a gmail account specifically created for this purpose only.
*/
#define GMAIL_USER		"username@domain.com"
#define GAMIL_PASSWORD	"password"
#define GMAIL_NICE_NAME	"The name of the sender" 

/* This is the one that will be receiveing all the raspberry mails */
#define GMAIL_TO_NOTIFY	"username@domain.com"
#define GMAIL_LOGGER	"The name of the receiver"

#endif 

#ifdef __cplusplus
}
#endif 