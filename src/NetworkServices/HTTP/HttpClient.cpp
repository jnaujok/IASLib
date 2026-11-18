/**
 *  HTTP Client Class
 *
 *      Sends HTTP/1.1 requests to one server over the library's client
 * sockets and reads the responses. See HttpClient.h.
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

#include "NetworkServices/HTTP/HttpClient.h"
#include "NetworkServices/HTTP/HttpUtils.h"
#include "Sockets/ClientSocket.h"
#include "Sockets/SocketConfig.h"
#ifdef IASLIB_TLS__
#include "Sockets/SecureClientSocket.h"
#endif

#include <vector>

namespace IASLib
{
    IMPLEMENT_OBJECT( CHttpClient, CGenericClient );

        // (The client was a socket connected in the constructor, and
        // executeRequest() built the request text and returned an empty
        // response without sending anything.)
    CHttpClient::CHttpClient( const char *strRemoteHost, int nHttpPort, bool bSecure )
        : CGenericClient( strRemoteHost, ( nHttpPort > 0 ) ? nHttpPort : ( bSecure ? 443 : 80 ), bSecure )
    {
        m_bUseHttp11 = true;
        m_bUseKeepalive = true;
        m_pSocket = NULL;
        m_nTimeoutMs = 30000;
        m_nMaxHeaderSize = 64 * 1024;
        m_nMaxBodySize = 64 * 1024 * 1024;
        m_nConnectionsOpened = 0;
        m_strUserAgent = "IASLib/2.0";
    }

    CHttpClient::~CHttpClient( void )
    {
        close();
    }

    void CHttpClient::setTlsContext( const std::shared_ptr<CTlsContext> &pContext )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        m_pTlsContext = pContext;
    }

    CString CHttpClient::getLastError( void )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        return m_strLastError;
    }

    bool CHttpClient::IsConnected( void )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        return ( m_pSocket != NULL ) && m_pSocket->IsConnected();
    }

    void CHttpClient::close( void )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        if ( m_pSocket )
        {
            m_pSocket->Close();
            delete m_pSocket;
            m_pSocket = NULL;
        }
        m_strPending.clear();
        m_bConnected = false;
        m_bSecureHandshakeComplete = false;
    }

    void CHttpClient::fail( const char *pszError )
    {
        m_strLastError = pszError;
        close();
    }

    CString CHttpClient::hostHeader( void )
    {
        CString strHost = m_strRemoteHost;
            // An IPv6 literal is written in brackets (RFC 3986 3.2.2).
        if ( strHost.IndexOf( ':' ) != NOT_FOUND && strHost.IndexOf( '[' ) == NOT_FOUND )
        {
            strHost = CString( "[" ) + strHost + "]";
        }
        if ( m_nPort != ( m_bSecure ? 443 : 80 ) )
        {
            strHost += CString::FormatString( ":%d", m_nPort );
        }
        return strHost;
    }

    bool CHttpClient::connect( void )
    {
        if ( m_pSocket && m_pSocket->IsConnected() )
        {
            return true;
        }
        close();

        if ( m_strRemoteHost.GetLength() == 0 )
        {
            m_strLastError = "No host to connect to.";
            return false;
        }

        if ( m_bSecure )
        {
#ifdef IASLIB_TLS__
            CSecureClientSocket *pSecure = m_pTlsContext ? new CSecureClientSocket( m_strRemoteHost, m_nPort, m_pTlsContext )
                                                         : new CSecureClientSocket( m_strRemoteHost, m_nPort );
            if ( ! pSecure->IsConnected() )
            {
                delete pSecure;
                m_strLastError = CString::FormatString( "Could not connect to %s:%d.", (const char *)m_strRemoteHost, m_nPort );
                return false;
            }
            if ( m_nTimeoutMs > 0 )
            {
                pSecure->SetSendTimeout( m_nTimeoutMs );
            }
            if ( ! pSecure->Handshake( ( m_nTimeoutMs > 0 ) ? m_nTimeoutMs : -1 ) )
            {
                m_strLastError = CString( "TLS handshake failed: " ) + pSecure->GetTlsError();
                pSecure->Close();
                delete pSecure;
                return false;
            }
            m_pSocket = pSecure;
            m_bSecureHandshakeComplete = true;
#else
            m_strLastError = "https needs a library built with OpenSSL (IASLIB_TLS__).";
            return false;
#endif
        }
        else
        {
                // The timeout also bounds connect() (as the send timeout) and
                // every send; reads use ReadWithTimeout().
            CSocketConfigBuilder builder;
            builder.setTCPNoDelay( true );
            if ( m_nTimeoutMs > 0 )
            {
                builder.setTimeout( m_nTimeoutMs );
            }
            CClientSocket *pSocket = new CClientSocket( builder.Build(), m_strRemoteHost, m_nPort );
            if ( ! pSocket->IsConnected() )
            {
                delete pSocket;
                m_strLastError = CString::FormatString( "Could not connect to %s:%d.", (const char *)m_strRemoteHost, m_nPort );
                return false;
            }
            m_pSocket = pSocket;
        }

        m_nConnectionsOpened++;
        m_bConnected = true;
        m_strPending.clear();
        return true;
    }

    bool CHttpClient::sendAll( const CString &strData )
    {
        try
        {
                // A plain socket throws on a failure; a secure one returns -1.
            int nSent = m_pSocket->Send( (const char *)strData, (int)strData.GetLength() );
            return nSent == (int)strData.GetLength();
        }
        catch ( ... )
        {
            return false;
        }
    }

    CHttpResponse *CHttpClient::readResponse( bool bHead, bool &bNothingRead )
    {
        std::string buffer;
        buffer.swap( m_strPending );
        std::vector<char> chunk( 16384 );
        bool bClosed = false;
        bNothingRead = buffer.empty();

        while ( true )
        {
            CHttpFrame frame = CHttpUtils::frameResponse( buffer.data(), buffer.size(), bHead, bClosed, m_nMaxHeaderSize, m_nMaxBodySize );

            if ( frame.status == CHttpFrame::FAILED )
            {
                if ( bClosed && buffer.empty() )
                {
                    fail( "The server closed the connection without a response." );
                }
                else if ( frame.errorStatus == 431 )
                {
                    fail( "The response's header section is too large." );
                }
                else if ( frame.errorStatus == 413 )
                {
                    fail( "The response's body is too large." );
                }
                else if ( bClosed )
                {
                    fail( "The server closed the connection before the response was complete." );
                }
                else
                {
                    fail( "The response is malformed." );
                }
                return NULL;
            }

            if ( frame.status == CHttpFrame::COMPLETE )
            {
                buffer.erase( 0, frame.consumed );

                    // Interim responses (100 Continue, 103 Early Hints) come
                    // before the real one (RFC 9110 15.2).
                if ( frame.statusCode >= 100 && frame.statusCode < 200 && frame.statusCode != 101 )
                {
                    continue;
                }

                CHttpResponse *pResponse = new CHttpResponse();
                if ( ! pResponse->parse( CString::FromBytes( frame.message.data(), frame.message.size() ) ) )
                {
                    delete pResponse;
                    fail( "The response is malformed." );
                    return NULL;
                }
                pResponse->setHeadResponse( bHead );

                    // Bytes after the response on a connection that is kept
                    // would be the start of an answer to a request not sent, so
                    // such a connection is not reused.
                if ( ! frame.keepAlive || ! m_bUseKeepalive || bClosed || ! buffer.empty() )
                {
                    close();
                }
                return pResponse;
            }

            if ( bClosed )
            {
                fail( "The server closed the connection before the response was complete." );
                return NULL;
            }

            int nRead = m_pSocket->ReadWithTimeout( chunk.data(), (int)chunk.size(), ( m_nTimeoutMs > 0 ) ? m_nTimeoutMs : -1 );
            if ( nRead == CSocket::READ_TIMEOUT )
            {
                fail( "Timed out waiting for the response." );
                return NULL;
            }
            if ( nRead < 0 )
            {
                if ( buffer.empty() )
                {
                    fail( "The connection failed before a response arrived." );
                }
                else
                {
                    fail( "The connection failed while reading the response." );
                }
                return NULL;
            }
            if ( nRead == 0 )
            {
                bClosed = true;     // Frame once more: the body may run to the close.
                continue;
            }
            bNothingRead = false;
            buffer.append( chunk.data(), (size_t)nRead );
        }
    }

    CHttpResponse *CHttpClient::sendRequest( CHttpRequest *request, const CString &strMessage, bool bHead, bool bRetry )
    {
        bool bReused = ( m_pSocket != NULL );
        if ( bReused )
        {
                // An idle connection the server has closed reads as closed (or
                // holds something unasked for): start afresh.
            if ( m_pSocket->HasData() )
            {
                close();
                bReused = false;
            }
        }
        if ( ! connect() )
        {
            return NULL;
        }

        if ( ! sendAll( strMessage ) )
        {
            fail( "Could not send the request." );
            if ( bReused && bRetry )
            {
                return sendRequest( request, strMessage, bHead, false );
            }
            return NULL;
        }

        bool bNothingRead = false;
        CHttpResponse *pResponse = readResponse( bHead, bNothingRead );
        if ( pResponse == NULL && bReused && bRetry && bNothingRead )
        {
                // RFC 9112 9.3.1: a request sent on a connection the server
                // closed meanwhile may be retried, if it is idempotent.
            return sendRequest( request, strMessage, bHead, false );
        }
        return pResponse;
    }

    CGenericResponse *CHttpClient::executeRequest( CGenericRequest *request )
    {
        CHttpRequest *pHttpRequest = dynamic_cast<CHttpRequest *>( request );
        if ( pHttpRequest == NULL )
        {
            std::lock_guard<std::recursive_mutex> lock( m_mutex );
            m_strLastError = "Not an HTTP request.";
            return NULL;
        }
        return executeRequest( pHttpRequest );
    }

    CHttpResponse *CHttpClient::executeRequest( CHttpRequest *request )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        m_strLastError = "";

        if ( request == NULL )
        {
            m_strLastError = "No request.";
            return NULL;
        }

            // The request line is checked again here, as a derived request
            // can set its members directly.
        CString method = request->getRequestType();
        if ( ! request->isValid() || ! CGenericRequest::isValidMethod( method ) || ! CGenericRequest::isValidUri( request->getUri() ) )
        {
            m_strLastError = "The request's method or URI cannot be sent (empty, or holding spaces or control characters).";
            return NULL;
        }

        request->setVersion( m_bUseHttp11 ? "HTTP/1.1" : "HTTP/1.0" );
        if ( ! request->hasHeader( "Host" ) )
        {
            request->setHeaderValue( "Host", hostHeader() );
        }
        if ( m_strUserAgent.GetLength() > 0 && ! request->hasHeader( "User-Agent" ) )
        {
            request->setHeaderValue( "User-Agent", m_strUserAgent );
        }
        if ( ! request->hasHeader( "Connection" ) )
        {
            if ( ! m_bUseKeepalive )
            {
                request->setHeaderValue( "Connection", "close" );
            }
            else if ( ! m_bUseHttp11 )
            {
                request->setHeaderValue( "Connection", "keep-alive" );
            }
        }

        CString strMessage = request->toString();
        bool bHead = ( method == CHttpRequest::METHOD_HEAD );
        bool bIdempotent = ( method == "GET" || method == "HEAD" || method == "PUT" || method == "DELETE" ||
                             method == "OPTIONS" || method == "TRACE" );

        return sendRequest( request, strMessage, bHead, bIdempotent );
    }

    CHttpResponse *CHttpClient::get( const char *uri )
    {
        CHttpRequest request( CHttpRequest::METHOD_GET, uri );
        return executeRequest( &request );
    }

    CHttpResponse *CHttpClient::head( const char *uri )
    {
        CHttpRequest request( CHttpRequest::METHOD_HEAD, uri );
        return executeRequest( &request );
    }

    CHttpResponse *CHttpClient::remove( const char *uri )
    {
        CHttpRequest request( CHttpRequest::METHOD_DELETE, uri );
        return executeRequest( &request );
    }

    CHttpResponse *CHttpClient::post( const char *uri, const char *contentType, const CString &body )
    {
        return withBody( CHttpRequest::METHOD_POST, uri, contentType, body );
    }

    CHttpResponse *CHttpClient::put( const char *uri, const char *contentType, const CString &body )
    {
        return withBody( CHttpRequest::METHOD_PUT, uri, contentType, body );
    }

    CHttpResponse *CHttpClient::withBody( const char *method, const char *uri, const char *contentType, const CString &body )
    {
        CHttpRequest request( method, uri );
        CString strType( ( contentType && *contentType ) ? contentType : "application/octet-stream" );
        if ( ! request.setHeaderValue( "Content-Type", strType ) )
        {
            std::lock_guard<std::recursive_mutex> lock( m_mutex );
            m_strLastError = "The content type cannot be sent (it holds CR, LF or NUL).";
            return NULL;
        }
        request.setEntity( CEntity::getEntity( strType, body ) );
        return executeRequest( &request );
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
