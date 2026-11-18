/**
 * @file HttpClient.h
 * @brief An HTTP/1.1 and HTTPS client for one server (CHttpClient).
 *
 *  Http Client Class
 *
 *      This class sends HTTP/1.1 requests to one server and reads the
 * responses (RFC 9110, RFC 9112). It is built on the library's socket
 * classes: a CClientSocket for http, and a CSecureClientSocket for https
 * (in a library built with OpenSSL, IASLIB_TLS__).
 *
 * @code
 *          CHttpClient client( "example.com", 80 );
 *          client.setTimeout( 5000 );
 *          CHttpResponse *response = client.get( "/index.html" );
 *          if ( response == NULL ) ... client.getLastError() ...
 *          else { ... response->getStatusCode() ... delete response; }
 * @endcode
 *
 *      For each request the client:
 *        - adds a Host header (and Connection: close when keep-alive is off)
 *          if the request has none;
 *        - refuses to send a request whose method, URI or version could not
 *          be sent as given (a CR or LF in the URI would add header lines);
 *        - reads the response with a body framed by Content-Length, by
 *          chunked transfer coding (decoded; the response then carries a
 *          Content-Length in place of Transfer-Encoding), or by the server
 *          closing the connection; responses to HEAD, and 1xx, 204 and 304
 *          responses, have no body; interim 1xx responses are skipped;
 *        - keeps the connection open for the next request when both sides
 *          allow it (HTTP/1.1 without "Connection: close", or HTTP/1.0 with
 *          "keep-alive"), and opens a new one when the server has closed it
 *          (an idempotent request that got no answer on a reused connection
 *          is sent once more on a new one).
 *
 *      Every wait - connecting, the TLS handshake, sending, and each read of
 * the response - is limited by the timeout (30 seconds by default). On a
 * failure executeRequest() returns NULL, getLastError() says why, and the
 * connection is closed.
 *
 *      A client is used by one thread at a time; calls from several threads
 * are serialised.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 8/16/2006
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPCLIENT_H__
#define IASLIB_HTTPCLIENT_H__

#include "NetworkServices/GenericClient.h"
#include "NetworkServices/HTTP/HttpRequest.h"
#include "NetworkServices/HTTP/HttpResponse.h"
#include "Sockets/Socket.h"

#include <memory>
#include <mutex>
#include <string>

namespace IASLib
{
    class CTlsContext;

    /// Sends HTTP/1.1 requests to one server and reads the responses
    /// (RFC 9110, RFC 9112), over plain TCP or TLS.
    ///
    /// Nothing is connected until the first request. The client keeps the
    /// connection open between requests when both sides allow it, reconnects
    /// when the server has closed it, and limits every wait by a timeout. On
    /// a failure the request call returns NULL, getLastError() says why, and
    /// the connection is closed.
    ///
    /// Each returned CHttpResponse is owned by the caller. Every public call
    /// takes the client's mutex, so calls from several threads are
    /// serialised; a client still talks to the server one request at a time.
    /// @code
    /// CHttpClient client( "example.com", 0, true );     // https, port 443
    /// client.setTimeout( 5000 );
    /// CHttpResponse *pResponse = client.post( "/api/items", "application/json", "{\"name\":\"x\"}" );
    /// if ( pResponse == NULL )
    ///     printf( "%s\n", (const char *)client.getLastError() );
    /// else
    /// {
    ///     int nStatus = pResponse->getStatusCode();
    ///     delete pResponse;
    /// }
    /// @endcode
    class CHttpClient : public CGenericClient
    {
        protected:
            bool                        m_bUseHttp11;           ///< Send HTTP/1.1 (true, the default) or HTTP/1.0.
            bool                        m_bUseKeepalive;        ///< Keep connections open between requests (true by default).
            CSocket                    *m_pSocket;              ///< The open connection (owned), or NULL.
            std::string                 m_strPending;           ///< Bytes read past the last response.
            int                         m_nTimeoutMs;           ///< The limit on each wait, in milliseconds (30000 by default).
            size_t                      m_nMaxHeaderSize;       ///< The largest response header section accepted (64 KB by default).
            size_t                      m_nMaxBodySize;         ///< The largest response body accepted (64 MB by default).
            size_t                      m_nConnectionsOpened;   ///< How many connections have been opened in all.
            CString                     m_strUserAgent;         ///< The User-Agent added to requests ("IASLib/2.0" by default; "" for none).
            CString                     m_strLastError;         ///< Why the last request failed, or "".
            std::shared_ptr<CTlsContext> m_pTlsContext;         ///< The TLS context for https, or empty for the default.
            std::recursive_mutex        m_mutex;                ///< Serialises the public calls.

        public:
                /// Creates a client for one server. Nothing is connected until
                /// the first request.
                /// @param strRemoteHost The server's host name or address.
                /// @param nHttpPort     The port; 0 means the scheme's default
                ///                      (80, or 443 when bSecure).
                /// @param bSecure       Whether to use https (TLS).
                /// @note The client used to be a socket connected in the
                ///       constructor, and executeRequest() built the request
                ///       text and returned an empty response without sending
                ///       anything.
	                                    CHttpClient( const char *strRemoteHost, int nHttpPort=80, bool bSecure = false );
	            /// Closes the connection, if one is open.
	        virtual                    ~CHttpClient( void );

                                        DEFINE_OBJECT( CHttpClient )

                                        /// Not copyable.
                                        CHttpClient( const CHttpClient & ) = delete;
                                        /// Not assignable.
            CHttpClient                &operator =( const CHttpClient & ) = delete;

                /// Sends the request and reads its response.
                ///
                /// Same as the CHttpRequest overload, for a request known only as
                /// a CGenericRequest.
                /// @return The response, which the caller deletes, or NULL on a
                ///         failure (see getLastError()) or for a request that is
                ///         not a CHttpRequest.
            virtual CGenericResponse   *executeRequest( CGenericRequest *request );
                /// Sends the request and reads its response.
                ///
                /// The request is changed first: its version is set to HTTP/1.1
                /// (or HTTP/1.0), and it gets Host, User-Agent and Connection
                /// ("close" without keep-alive, "keep-alive" for HTTP/1.0)
                /// headers where it has none. A request that is not valid, or
                /// whose method or URI could not be sent as given, is refused.
                ///
                /// Interim 1xx responses (other than 101) are skipped. A request
                /// that is idempotent (GET, HEAD, PUT, DELETE, OPTIONS, TRACE)
                /// and got no answer on a reused connection is sent once more on
                /// a new one (RFC 9112 9.3.1).
                /// @return The response, which the caller deletes, or NULL on a
                ///         failure (see getLastError()).
            virtual CHttpResponse      *executeRequest( CHttpRequest *request );

                /// Sends a GET request for the URI.
                /// @return The response (caller deletes it), or NULL on a failure.
            virtual CHttpResponse      *get( const char *uri );
                /// Sends a HEAD request for the URI. The response has no body.
                /// @return The response (caller deletes it), or NULL on a failure.
            virtual CHttpResponse      *head( const char *uri );
                /// Sends a POST request with a body.
                /// @param uri         The request target.
                /// @param contentType The body's Content-Type; NULL or "" means
                ///                    "application/octet-stream".
                /// @param body        The body bytes.
                /// @return The response (caller deletes it), or NULL on a failure,
                ///         including a content type holding CR, LF or NUL.
            virtual CHttpResponse      *post( const char *uri, const char *contentType, const CString &body );
                /// Sends a PUT request with a body; as post().
            virtual CHttpResponse      *put( const char *uri, const char *contentType, const CString &body );
                /// Sends a DELETE request for the URI.
                /// @return The response (caller deletes it), or NULL on a failure.
            virtual CHttpResponse      *remove( const char *uri );

                /// Sets the limit on each wait (connecting, the TLS handshake,
                /// sending, and each read), in milliseconds (0 or less: wait as
                /// long as it takes). It applies to connections opened after
                /// the call.
            void                        setTimeout( int nMilliseconds ) { m_nTimeoutMs = nMilliseconds; }
                /// The limit on each wait, in milliseconds.
            int                         getTimeout( void ) const { return m_nTimeoutMs; }

                /// Sets whether to keep connections open between requests (on by
                /// default).
            void                        setKeepAlive( bool bKeepAlive ) { m_bUseKeepalive = bKeepAlive; }
                /// Whether connections are kept open between requests.
            bool                        getKeepAlive( void ) const { return m_bUseKeepalive; }

                /// Sends requests as HTTP/1.1 (true, the default) or, when false,
                /// as HTTP/1.0.
            void                        setUseHttp11( bool bUseHttp11 ) { m_bUseHttp11 = bUseHttp11; }

                /// Sets the limit on a response's header section (default 64 KB);
                /// a larger one fails.
            void                        setMaxHeaderSize( size_t nBytes ) { m_nMaxHeaderSize = nBytes; }
                /// Sets the limit on a response's body (default 64 MB); a larger
                /// one fails.
            void                        setMaxBodySize( size_t nBytes ) { m_nMaxBodySize = nBytes; }

                /// Sets the User-Agent header added to requests that have none
                /// (default "IASLib/2.0"; "" or NULL for none).
            void                        setUserAgent( const char *pszAgent ) { m_strUserAgent = pszAgent ? pszAgent : ""; }

                /// Sets the client TLS context for https (by default, one that
                /// trusts the system's certificate authorities).
                ///
                /// Call it before the first request. Ignored in a library built
                /// without TLS.
            void                        setTlsContext( const std::shared_ptr<CTlsContext> &pContext );

                /// Why the last request failed ("" if it did not).
            CString                     getLastError( void );

                /// Whether a connection is open now.
            virtual bool                IsConnected( void );
                /// How many connections have been opened in all (to see
                /// connections being reused).
            size_t                      getConnectionsOpened( void ) const { return m_nConnectionsOpened; }

                /// Closes the connection, if one is open.
            virtual void                close( void );

                /// The Host header value for the server: the host, in brackets
                /// if it is an IPv6 address, with the port unless it is the
                /// scheme's default.
            virtual CString             hostHeader( void );

        protected:
                /// Opens a connection unless one is open, doing the TLS handshake
                /// for https.
                /// @return false, with m_strLastError set, if it could not.
            bool                        connect( void );
                /// Sends all of the data on the connection.
                /// @return false if it could not all be sent.
            bool                        sendAll( const CString &strData );
                /// Reads one response.
                ///
                /// Closes the connection afterwards unless it may be reused.
                /// @param bHead        Whether the request was HEAD.
                /// @param bNothingRead Set when the connection closed before a
                ///                     single byte of the response arrived.
                /// @return The response (caller deletes it), or NULL on a failure.
            CHttpResponse              *readResponse( bool bHead, bool &bNothingRead );
                /// Connects if needed, sends the message and reads the response.
                /// @param request    The request (passed on to a retry).
                /// @param strMessage The serialized request.
                /// @param bHead      Whether the request is HEAD.
                /// @param bRetry Whether the request may be sent once more on a
                ///               new connection if a reused one fails.
                /// @return The response (caller deletes it), or NULL on a failure.
            CHttpResponse              *sendRequest( CHttpRequest *request, const CString &strMessage, bool bHead, bool bRetry );
                /// Records the error and closes the connection.
            void                        fail( const char *pszError );
                /// Builds and sends a request with a body; used by post() and put().
            CHttpResponse              *withBody( const char *method, const char *uri, const char *contentType, const CString &body );
    };
} // namespace IASLib

#endif // IASLIB_HTTPCLIENT_H__

#endif // IASLIB_NETWORKING__
