#ifndef _CMAIL_H
#define _CMAIL_H

#include <stdbool.h>

typedef struct cmail cmail;
typedef struct cmail_sender cmail_sender;
typedef struct cmail_email cmail_email;
typedef struct cmail_credentials cmail_credentials;

cmail *cmail_init(void);

bool cmail_credentials_load(char *filename, cmail_credentials **creds);

cmail_credentials *cmail_credentials_create(char *auth_method, ...);

bool cmail_credentials_authorize(cmail *c, cmail_credentials *creds);

void cmail_credentials_save(cmail_credentials *creds, char *filename);

cmail_sender *cmail_sender_create(char *server_url, char *sender_address, cmail_credentials *creds);

cmail_email *cmail_email_new(void);

void cmail_email_add_recipient(cmail_email *email, char *address);

void cmail_email_add_header(cmail_email *email, char *header, char *value);

#endif