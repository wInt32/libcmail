# libcmail
A better way to do emails from C.

### Planned features
- sending plaintext/HTML emails with attachments through a SMTP(S) server (work in progress)
- support for different SMTP(S) servers: Google (gmail), Microsoft, custom oauth2 servers...
- more will be added soon!

### Building
- building is currently only supported on Linux (probably works on other 'nixes though)
- external dependencies: libcurl, cJSON (bundled with libcmail)

To build libcmail for Linux you can just use `make`:
```sh
$ make
```
- by default it will build the sources tarball and release zip
- all build artifacts are stored in the `target/` directory

To build for Windows (win32), specify the target variable:
```sh
$ make target=win32
```
A debug configuration is also available:
```sh
$ make debug=1
```
this will build the examples, not the release archives!

#### Other make targets:
- `make clean` - removes any build artifacts
- `make release` - same as the default, cannot be built in debug mode
- `make examples` - builds the examples, needs `examples/credentials.txt`

### License
This library is licensed under the MIT license, of which a copy is available in the LICENSE file.

### Learning resources
- [Using OAuth 2.0 to Access Google APIs](https://developers.google.com/identity/protocols/oauth2)
- [OAuth 2.0 Mechanism](https://developers.google.com/gmail/imap/xoauth2-protocol)
- [Authenticate an IMAP, POP or SMTP connection using OAuth](https://learn.microsoft.com/en-us/exchange/client-developer/legacy-protocols/how-to-authenticate-an-imap-pop-smtp-application-by-using-oauth)
- [Simple Mail Transfer Protocol](https://datatracker.ietf.org/doc/html/rfc5321)
