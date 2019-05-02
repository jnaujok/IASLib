/**
 *  Secure Socket Class
 *
 *      This class provides platform independent support for TCP/IP
 * sockets that make use of SSL or TLS encryption to wrap the socket for
 * secure transactions. This is done in a transparent manner to allow the
 * same higher level code to use either secure or non-secure socket
 * communication without changes to the code.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/22/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__
#include "SecureSocket.h"
#include "SocketException.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace IASLib
{
static bool g_bSSLInitialized = false;

IMPLEMENT_OBJECT(CSecureSocket, CSocket);

    /**
     *  Pre-existing Socket Constructor
     *
     *      This constructor takes an already existing socket and wraps it
     * in a class. This is especially useful for sockets resulting from
     * accept calls.
     *
     * @param hSocket
     *          The handle of the socket that already exists.
     * @param strSockName
     *          A text name to assign to the socket. Informational only.
     * @param AddressIn
     *          A pointer to the sockaddr_in structure that includes the
     *          address information for the socket being attached.
     */
CSecureSocket::CSecureSocket(SOCKET hSocket, const char *strSockName, void *AddressIn) : CSocket(hSocket, strSockName, AddressIn)
{
    // Build the secure context around the socket
    if (!g_bSSLInitialized)
    {
        initSsl();
        g_bSSLInitialized = true;
    }

    m_pContext = createContext();
    configureContext( m_pContext );
}

/**
     *  Server Socket Constructor
     *
     *      This constructor creates and binds a port for listening and
     * accepting connections from a client.
     *
     * @param nPort
     *          The Berkeley Sockets port to connect to, from 0-65535.
     * @param bBlocking
     *          Is the port a blocking port, or does the call to listen
     *          return immediately.
     */
CSecureSocket::CSecureSocket(int nPort, bool bBlocking) : CSocket(nPort, bBlocking)
{
    // Note, server sockets don't have a context -- they will accept
    // connections and then wrap those sockets in security.
    m_pContext = NULL;
}

/**
     *  Client Socket Constructor
     *
     *      This constructor takes a remote host and a port to connect to
     * and then attempts to connect to that host via that port.
     *
     * @param strConnectTo
     *          The host name or IP address to connect to.
     * @param nPort
     *          The remote port to connect to.
     */
CSecureSocket::CSecureSocket(const char *strConnectTo, int nPort) : CSocket(strConnectTo, nPort)
{
}

/**
     *  Socket Destructor
     *
     *      This method is used to clean up a socket when the object is
     * being destroyed.
     */
CSecureSocket::~CSecureSocket()
{
}

void CSecureSocket::initSsl()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

SSL_CTX *CSecureSocket::createContext()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx)
    {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void CSecureSocket::configureContext(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);

    // Set the key and cert
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

}; // namespace IASLib

#endif

/*
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

int create_socket(int port)
{
    int s;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
	perror("Unable to create socket");
	exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
	perror("Unable to bind");
	exit(EXIT_FAILURE);
    }

    if (listen(s, 1) < 0) {
	perror("Unable to listen");
	exit(EXIT_FAILURE);
    }

    return s;
}


void cleanup_openssl()
{
    EVP_cleanup();
}


int main(int argc, char **argv)
{
    int sock;
    SSL_CTX *ctx;

    init_openssl();
    ctx = create_context();

    configure_context(ctx);

    sock = create_socket(4433);

    //  Handle connections
    while(1) {
        struct sockaddr_in addr;
        uint len = sizeof(addr);
        SSL *ssl;
        const char reply[] = "test\n";

        int client = accept(sock, (struct sockaddr*)&addr, &len);
        if (client < 0) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        }
        else {
            SSL_write(ssl, reply, strlen(reply));
        }

        SSL_free(ssl);
        close(client);
    }

    close(sock);
    SSL_CTX_free(ctx);
    cleanup_openssl();
}
*/
