#include <stdlib.h>

#ifndef CMAIL_BUILD
#include "../src/cmail.h"
#else
#include <cmail.h> // include <cmail.h>, link with libcmail.a and libcurl
#endif

int main() {

    // initializes the library
    cmail *cmail = cmail_init();

    cmail_credentials *credentials = NULL;

    // load credentials from a file or create new
    if (!cmail_credentials_load("test.txt", &credentials)) {
        credentials = cmail_credentials_create(
            "OAUTH2", // authentication method

            /* method specific credentials */
            "username@example.com", // OAUTH2 username (usually same as email address)
            "client-id", // OAUTH2 client ID
            "client-secret", // OAUTH2 client secret
            NULL, // authorization code
            NULL, // refresh token
            NULL // access token
        );
        cmail_credentials_authorize(cmail, credentials);
        cmail_credentials_save(credentials, "test.txt");
    }

    // creates a new email sender, connects to the server and authenticates
    cmail_sender *sender = cmail_sender_create(
        "smtps://mail.example.com", // email server URL
        "example@example.com", // sender email address
        credentials
    );

    // creates an empty email
    cmail_email *email = cmail_email_new();

    // adds a recipient to the email
    cmail_email_add_recipient(email, "example@example.com");

    // adds a header to the email
    cmail_email_add_header(email, "Header", "header content");

}