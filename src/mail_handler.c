#include "mail_handler.h"

static char base_wd[256];
static char this_name[256], this_mail[256], this_password[256]; 

void init_mail_handler(const char* sender_name,const char* sender_mail,const char* password)
{
	if ( strstr(base_wd, "raspsite") == NULL ) {
		// Refresh
		memset(this_name, '\0', sizeof this_name); 
		memset(this_mail, '\0', sizeof this_mail);
		memset(this_password, '\0', sizeof this_password);

		snprintf(this_name, sizeof this_name, "%s", sender_name);
		snprintf(this_mail, sizeof this_mail, "%s", sender_mail);
		snprintf(this_password, sizeof this_password, "%s", password);

		snprintf(base_wd, sizeof base_wd, "%s", getenv("PWD"));
	}

}

void send_mail(const char* receiver_name, const char* receiver_mail, const char* subject_line, 
	int type, const char * data)
{
	char *command;
	char file_name[256];

	command = calloc(2056,1);
	memset(file_name, '\0', sizeof file_name);

	switch(type){
		case AUTH_SUCCESS_MAIL:
			snprintf(command, 2056, ". %s%s %s > %s%s", base_wd, SCRIPT_AUTH_PATH, data, base_wd, MAIL_AUTH_TXT_PATH);
			system(command);
			printf("Command: %s\n", command);
			snprintf(file_name, sizeof file_name, "%s%s", base_wd, MAIL_AUTH_TXT_PATH);
			break;
		case STATUS_MAIL:
			snprintf(command, 2056, ". %s%s > %s%s", base_wd, SCRIPT_STATUS_PATH, base_wd, MAIL_STATUS_TXT_PATH);
			system(command);
			snprintf(file_name, sizeof file_name, "%s%s", base_wd, MAIL_STATUS_TXT_PATH);
			break;
		default:
			snprintf(command, 2056, ". %s%s > %s%s", base_wd, SCRIPT_STATUS_PATH, base_wd, MAIL_STATUS_TXT_PATH);
			system(command);
			snprintf(file_name, sizeof file_name, "%s%s", base_wd, MAIL_STATUS_TXT_PATH);
			break;
	}

	snprintf(command, 2056, ". %s%s \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" &", \
		base_wd, SCRIPT_SEND_PATH, receiver_name, receiver_mail, subject_line, this_password, \
		file_name, this_name, this_mail );

	system(command);

	free(command);
}