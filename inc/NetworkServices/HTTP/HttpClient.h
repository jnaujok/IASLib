/**
 *  Http Client Class
 *
 *      This class sends HTTP/1.1 requests to one server and reads the
 * responses (RFC 9110, RFC 9112). It is built on the library's socket
 * classes: a CClientSocket for http, and a CSecureClientSocket for https
 * (in a library built with OpenSSL, IASLIB_TLS__).
 *
 *          CHttpClient client( "example.com", 80 );
 *          client.setTimeout( 5000 );
 *          CHttpResponse *response = client.get( "/index.html" );
 *          if ( response == NULL ) ... client.getLastError() ...
 *          else { ... response->getStatusCode() ... delete response; }
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

    class CHttpClient : public CGenericClient
    {
        protected:
            bool                        m_bUseHttp11;
            bool                        m_bUseKeepalive;
            CSocket                    *m_pSocket;          // the open connection, or NULL
            std::string                 m_strPending;       // bytes read past the last response
            int                         m_nTimeoutMs;
            size_t                      m_nMaxHeaderSize;
            size_t                      m_nMaxBodySize;
            size_t                      m_nConnectionsOpened;
            CString                     m_strUserAgent;
            CString                     m_strLastError;
            std::shared_ptr<CTlsContext> m_pTlsContext;
            std::recursive_mutex        m_mutex;

        public:
                // nHttpPort 0 means the scheme's default (80, or 443 when
                // bSecure). Nothing is connected until the first request.
	                                    CHttpClient( const char *strRemoteHost, int nHttpPort=80, bool bSecure = false );
	        virtual                    ~CHttpClient( void );

                                        DEFINE_OBJECT( CHttpClient )

                                        CHttpClient( const CHttpClient & ) = delete;
            CHttpClient                &operator =( const CHttpClient & ) = delete;

                // Sends the request and reads its response, which the caller
                // deletes. NULL on a failure (see getLastError()), or for a
                // request that is not a CHttpRequest. The request gets a Host
                // (and, without keep-alive, a Connection) header if it has
                // none.
            virtual CGenericResponse   *executeRequest( CGenericRequest *request );
            virtual CHttpResponse      *executeRequest( CHttpRequest *request );

                // Shorthands for the common methods.
            virtual CHttpResponse      *get( const char *uri );
            virtual CHttpResponse      *head( const char *uri );
            virtual CHttpResponse      *post( const char *uri, const char *contentType, const CString &body );
            virtual CHttpResponse      *put( const char *uri, const char *contentType, const CString &body );
            virtual CHttpResponse      *remove( const char *uri );

                // The limit on each wait, in milliseconds (0 or less: wait as
                // long as it takes).
            void                        setTimeout( int nMilliseconds ) { m_nTimeoutMs = nMilliseconds; }
            int                         getTimeout( void ) const { return m_nTimeoutMs; }

                // Whether to keep connections open between requests (on by
                // default).
            void                        setKeepAlive( bool bKeepAlive ) { m_bUseKeepalive = bKeepAlive; }
            bool                        getKeepAlive( void ) const { return m_bUseKeepalive; }

                // Sends requests as HTTP/1.0 instead of HTTP/1.1.
            void                        setUseHttp11( bool bUseHttp11 ) { m_bUseHttp11 = bUseHttp11; }

                // Limits on a response's header section (default 64 KB) and
                // body (default 64 MB); a larger one fails.
            void                        setMaxHeaderSize( size_t nBytes ) { m_nMaxHeaderSize = nBytes; }
            void                        setMaxBodySize( size_t nBytes ) { m_nMaxBodySize = nBytes; }

                // The User-Agent header added to requests that have none ("" for
                // none).
            void                        setUserAgent( const char *pszAgent ) { m_strUserAgent = pszAgent ? pszAgent : ""; }

                // https: the client TLS context to use (by default, one that
                // trusts the system's certificate authorities). Before the
                // first request. Ignored in a library built without TLS.
            void                        setTlsContext( const std::shared_ptr<CTlsContext> &pContext );

                // Why the last request failed ("" if it did not).
            CString                     getLastError( void );

                // Whether a connection is open now, and how many have been
                // opened in all (to see connections being reused).
            virtual bool                IsConnected( void );
            size_t                      getConnectionsOpened( void ) const { return m_nConnectionsOpened; }

                // Closes the connection, if one is open.
            virtual void                close( void );

                // The Host header value for the server: the host, in brackets
                // if it is an IPv6 address, with the port unless it is the
                // scheme's default.
            virtual CString             hostHeader( void );

        protected:
            bool                        connect( void );
            bool                        sendAll( const CString &strData );
                // Reads one response. bNothingRead is set when the connection
                // closed before a single byte of it arrived.
            CHttpResponse              *readResponse( bool bHead, bool &bNothingRead );
            CHttpResponse              *sendRequest( CHttpRequest *request, const CString &strMessage, bool bHead, bool bRetry );
            void                        fail( const char *pszError );
            CHttpResponse              *withBody( const char *method, const char *uri, const char *contentType, const CString &body );
    };
} // namespace IASLib

#endif // IASLIB_HTTPCLIENT_H__

#endif // IASLIB_NETWORKING__
