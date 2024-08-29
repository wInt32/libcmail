#include <stdlib.h>
#include <stdio.h>

#ifndef CMAIL_BUILD
#include "../src/cmail.h"
#else
#include <cmail.h> // include <cmail.h>, link with libcmail.a and libcurl
#endif

#include "credentials.txt"

int main() {

    // initializes the library
    cmail *cmail = cmail_init();

    cmail_credentials *credentials = cmail_credentials_create(
        cmail_OAUTH2 , // authentication method
        "google", // authentication provider

        /* method specific credentials */
        user_email_address, // OAUTH2 username (usually same as email address)
        client_id, // OAUTH2 client ID
        client_secret, // OAUTH2 client secret
        NULL, // authorization code
        NULL, // refresh token
        NULL // access token
    );
    if (cmail_credentials_oauth2_needs_auth(cmail, credentials)) {
        cmail_credentials_oauth2_authorize(cmail, credentials);
    }
    if (!cmail_credentials_oauth2_refresh(cmail, credentials)) {
        fprintf(stderr, "%s:%zu: could not refresh credentials\n", __FILE__, __LINE__);
        exit(-1);
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

    // send the email
    cmail_sender_send_email(cmail, sender, email);

}