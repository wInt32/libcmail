#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <curl/curl.h>

#include "cmail.h"


#define NEW(x) calloc(1, sizeof(x))

/* safely duplicate a string */
static char *dupstr(const char *src) {
    if (src == NULL) return NULL;
    size_t sz = strlen(src) + 1;
    char *dup = malloc(sz);
    assert(dup != NULL);
    memcpy(dup, src, sz);
    return dup;
}

/* append s1 and s2 to a new string */
static char *append_str(const char *s1, const char *s2) {
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *dst = malloc(len1 + len2 + 1);
    assert(dst != NULL);
    memcpy(dst, s1, len1);
    memcpy(dst+len1, s2, len2+1);
    return dst;
}

struct cmail {
    CURL *curl;
};

cmail *cmail_init() {
    cmail *c = NEW(cmail);
    c->curl = curl_easy_init();
    return c;
}

/* free cmail handle */
void cmail_free(cmail *c) {
    curl_easy_cleanup(c->curl);
    free(c);
}

struct cmail_credentials {
    cmail_auth_method auth_method;
    char *auth_provider;
    union {
    struct {
        char *username;
        char *client_id;
        char *client_secret;
        char *authorization_code;
        char *access_token;
        char *refresh_token;
    } oauth2;
    };
};

struct cmail_sender {
    char *server_url;
    char *address;
    cmail_credentials *creds;
};

struct cmail_email {

};

cmail_credentials *cmail_credentials_create(cmail_auth_method auth_method, const char *auth_provider, ...) {
    cmail_credentials *creds = NEW(cmail_credentials);
    creds->auth_method = auth_method;
    creds->auth_provider = dupstr(auth_provider);
    va_list ap;
    va_start(ap, auth_provider);
    creds->oauth2.username = dupstr(va_arg(ap, char *));
    creds->oauth2.client_id = dupstr(va_arg(ap, char *));
    creds->oauth2.client_secret = dupstr(va_arg(ap, char *));
    creds->oauth2.authorization_code = dupstr(va_arg(ap, char *));
    creds->oauth2.access_token = dupstr(va_arg(ap, char *));
    creds->oauth2.refresh_token = dupstr(va_arg(ap, char *));
    va_end(ap);
    return creds;
}

void cmail_credentials_oauth2_set_access_token(cmail_credentials *creds, const char *access_token) {
    if (creds->oauth2.access_token != NULL)
        free(creds->oauth2.access_token);
    creds->oauth2.access_token = strdup(access_token);
}

void cmail_credentials_oauth2_set_refresh_token(cmail_credentials *creds, const char *refresh_token) {
    if (creds->oauth2.refresh_token != NULL)
        free(creds->oauth2.refresh_token);
    creds->oauth2.refresh_token = strdup(refresh_token);
}

bool cmail_credentials_oauth2_needs_auth(cmail *c, cmail_credentials *creds) {
    if (creds->oauth2.refresh_token == NULL)
        return true;
    if (creds->oauth2.access_token == NULL)
        return true;
    return false;
}

bool listen_and_fetch_auth_code(cmail *c, cmail_credentials *creds) {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(12000);
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 1);

    struct sockaddr_in client_addr;
    socklen_t client_addr_sz = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_sz);
    puts("connection accepted!");

    char buf[4096] = {0};

    if (recv(client_fd, buf, 4095, 0) < 0) {
        return false;
    }

    const char headers[] = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
    const char success_page[] = "<html><body><h1>Authenticated.</h1></body></html>\r\n";
    const char error_page[] = "<html><body><h1>Authentication error.</h1></body></html>\r\n";

    char *pattern = "GET /?code=%[^&]s";
    char *authorization_code = calloc(256, 1);

    if (authorization_code == NULL) {
        send(client_fd, error_page, sizeof(error_page)-1, 0);
        goto cleanup;
    }

    sscanf(buf, pattern, authorization_code);
    send(client_fd, headers, sizeof(headers)-1, 0);
    if (authorization_code[0] == 0) {
        send(client_fd, error_page, sizeof(error_page)-1, 0);
        free(authorization_code);
        authorization_code = NULL;
        goto cleanup;
    }
    send(client_fd, success_page, sizeof(success_page)-1, 0);
    creds->oauth2.authorization_code = authorization_code;
    return true;
cleanup:
    return false;

}

/* authorize once */
bool cmail_credentials_oauth2_authorize(cmail *c, cmail_credentials *creds) {
    #ifdef __linux__
        #define COMMAND "xdg-open \""
    #endif
    #ifdef __APPLE__
        #define COMMAND "open \""
    #endif
    #if defined(_WIN32)
        #define COMMAND "start \"\" \""
    #endif
    char *cmd_base = COMMAND;
    if (strcmp(creds->auth_provider, "google") == 0) {
        cmd_base = append_str(cmd_base, "https://accounts.google.com/o/oauth2/v2/auth?scope=https://mail.google.com/&response_type=code&redirect_uri=http://localhost:12000&client_id=%s\"");
    }

    /* %s ... -2, \0 ... +1 */
    size_t cmd_sz = strlen(cmd_base)-2+strlen(creds->oauth2.client_id)+1;
    char *cmd = calloc(cmd_sz, 1);
    snprintf(cmd, cmd_sz, cmd_base, creds->oauth2.client_id);

    puts(cmd);

    /* simply atrocious */
    system(cmd);
    listen_and_fetch_auth_code(c, creds);
}

/* refresh when credentials expire */
bool cmail_credentials_oauth2_refresh(cmail *c, cmail_credentials *creds) {

}

cmail_sender *cmail_sender_create(char *server_url, char *sender_address, cmail_credentials *creds) {

}

cmail_email *cmail_email_new(void) {

}

void cmail_email_add_recipient(cmail_email *email, char *address) {

}

void cmail_email_add_header(cmail_email *email, char *header, char *value) {
    
}

bool cmail_sender_send_email(cmail *c, cmail_sender *sender, cmail_email *email) {

}