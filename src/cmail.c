#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <curl/curl.h>

#include "cmail.h"

struct cmail {
    CURL *curl;
};

struct cmail_sender {};
struct cmail_email {};
struct cmail_credentials {};

cmail *cmail_init() {
    cmail *c = calloc(1, sizeof(cmail));
    c->curl = curl_easy_init();
    return c;
}

bool cmail_credentials_load(char *filename, cmail_credentials **creds) {

}

cmail_credentials *cmail_credentials_create(char *auth_method, ...) {

}

bool cmail_credentials_authorize(cmail *c, cmail_credentials *creds) {

}

void cmail_credentials_save(cmail_credentials *creds, char *filename) {

}

cmail_sender *cmail_sender_create(char *server_url, char *sender_address, cmail_credentials *creds) {

}

cmail_email *cmail_email_new(void) {

}

void cmail_email_add_recipient(cmail_email *email, char *address) {

}

void cmail_email_add_header(cmail_email *email, char *header, char *value) {
    
}