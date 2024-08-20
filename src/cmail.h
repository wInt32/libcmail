#ifndef _CMAIL_H
#define _CMAIL_H

#include <stdbool.h>

typedef struct cmail cmail;
typedef struct cmail_sender cmail_sender;
typedef struct cmail_email cmail_email;
typedef struct cmail_credentials cmail_credentials;

typedef enum cmail_auth_method {
    cmail_OAUTH2
} cmail_auth_method;

cmail *cmail_init(void);

cmail_credentials *cmail_credentials_create(cmail_auth_method auth_method, const char *auth_provider, ...);

bool cmail_credentials_oauth2_needs_auth(cmail *c, cmail_credentials *creds);

bool cmail_credentials_oauth2_authorize(cmail *c, cmail_credentials *creds);

bool cmail_credentials_oauth2_refresh(cmail *c, cmail_credentials *creds);

cmail_sender *cmail_sender_create(char *server_url, char *sender_address, cmail_credentials *creds);

cmail_email *cmail_email_new(void);

void cmail_email_add_recipient(cmail_email *email, char *address);

void cmail_email_add_header(cmail_email *email, char *header, char *value);

bool cmail_sender_send_email(cmail *c, cmail_sender *sender, cmail_email *email);

#endif