#ifdef __cplusplus
extern "C" {
#endif 

#ifndef MAIL_HANDLER_H
#define MAIL_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SCRIPT_SEND_PATH		"/etc/mail/send_mail.sh"
#define SCRIPT_STATUS_PATH		"/etc/mail/status.sh"
#define SCRIPT_AUTH_PATH		"/etc/mail/authenticated.sh"

#define MAIL_STATUS_TXT_PATH	"/etc/mail/status.txt"
#define MAIL_AUTH_TXT_PATH		"/etc/mail/authenticated.txt"

#define AUTH_SUCCESS_MAIL	0
#define STATUS_MAIL			1

void init_mail_handler(const char*,const char*,const char*);
void send_mail(const char*,const char*,const char*, int, const char*);

#endif 

#ifdef __cplusplus
}
#endif 