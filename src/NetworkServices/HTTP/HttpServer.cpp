/**
 *  HTTP Server Class
 *
 *      The protocol core of an HTTP/1.1 server. See HttpServer.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: September 24, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "HttpServer.h"
#include "NetworkServices/HTTP/HttpUtils.h"
#include "Logging/LogSink.h"

#include <cctype>

namespace IASLib
{
    IMPLEMENT_OBJECT( CHttpServer, CGenericServer );

    CHttpServer::CHttpServer( int nHttpPort, bool bSecure ) : CGenericServer(), m_aHandlerFactories()
    {
        init();
        m_nPort = nHttpPort;
        m_bSecure = bSecure;
    }

    CHttpServer::CHttpServer( const char *boundInterface, int nHttpPort, bool bSecure ) : CGenericServer(), m_aHandlerFactories()
    {
        init();
        m_nPort = nHttpPort;
        m_bSecure = bSecure;
        m_strBindAddress = boundInterface ? boundInterface : "";
    }

    void CHttpServer::init( void )
    {
        m_bUseHTTP11 = true;
        m_bUseKeepalive = true;
        m_bSecure = false;
        m_nPort = 80;
        m_strServerName = "IASLib/2.0";
        m_nMaxHeaderSize = 64 * 1024;
        m_nMaxBodySize = 10 * 1024 * 1024;
        m_nKeepAliveTimeoutMs = 30000;
        m_nRequestTimeoutMs = 30000;
        m_bRequireMatchingHost = true;
    }

    CHttpServer::~CHttpServer( void )
    {
        m_aHandlerFactories.DeleteAll();
        for ( size_t nX = 0; nX < m_aVirtualHosts.size(); nX++ )
            for ( size_t nY = 0; nY < m_aVirtualHosts[ nX ].second.size(); nY++ )
                delete m_aVirtualHosts[ nX ].second[ nY ];
        m_aVirtualHosts.clear();
    }

    namespace
    {
            // A host name against a name or "*.domain" pattern (both already
            // normalized).
        bool hostMatches( const std::string &strPattern, const std::string &strHost )
        {
            if ( strPattern.size() > 2 && strPattern[0] == '*' && strPattern[1] == '.' )
            {
                std::string strSuffix = strPattern.substr( 1 );       // ".domain"
                if ( strHost.size() <= strSuffix.size() ||
                     strHost.compare( strHost.size() - strSuffix.size(), strSuffix.size(), strSuffix ) != 0 )
                    return false;
                return strHost.substr( 0, strHost.size() - strSuffix.size() ).find( '.' ) == std::string::npos;
            }
            return strPattern == strHost;
        }

        std::string normalizedPattern( const char *pszHostName )
        {
            std::string strName( pszHostName ? pszHostName : "" );
            if ( strName.compare( 0, 2, "*." ) == 0 )
                return "*." + std::string( (const char *)CHttpRequest::normalizeHostName( CString( strName.substr( 2 ).c_str() ) ) );
            return std::string( (const char *)CHttpRequest::normalizeHostName( CString( strName.c_str() ) ) );
        }
    }

    void CHttpServer::AddVirtualHost( const char *pszHostName, CHttpHandlerFactory *pFactory )
    {
        std::string strName = normalizedPattern( pszHostName );
        if ( pFactory == NULL || strName.empty() || strName == "*." )
        {
            delete pFactory;
            return;
        }
        for ( size_t nX = 0; nX < m_aVirtualHosts.size(); nX++ )
        {
            if ( m_aVirtualHosts[ nX ].first == strName )
            {
                m_aVirtualHosts[ nX ].second.insert( m_aVirtualHosts[ nX ].second.begin(), pFactory );
                return;
            }
        }
        m_aVirtualHosts.push_back( std::make_pair( strName, std::vector<CHttpHandlerFactory *>( 1, pFactory ) ) );
    }

    bool CHttpServer::RemoveVirtualHost( const char *pszHostName )
    {
        std::string strName = normalizedPattern( pszHostName );
        for ( size_t nX = 0; nX < m_aVirtualHosts.size(); nX++ )
        {
            if ( m_aVirtualHosts[ nX ].first == strName )
            {
                for ( size_t nY = 0; nY < m_aVirtualHosts[ nX ].second.size(); nY++ )
                    delete m_aVirtualHosts[ nX ].second[ nY ];
                m_aVirtualHosts.erase( m_aVirtualHosts.begin() + nX );
                return true;
            }
        }
        return false;
    }

    CString CHttpServer::findVirtualHost( const CString &strHostName )
    {
        std::string strHost( (const char *)CHttpRequest::normalizeHostName( strHostName ) );
        if ( strHost.empty() )
            return CString( "" );
            // Exact names first, then patterns.
        for ( size_t nX = 0; nX < m_aVirtualHosts.size(); nX++ )
            if ( m_aVirtualHosts[ nX ].first == strHost )
                return CString( m_aVirtualHosts[ nX ].first.c_str() );
        for ( size_t nX = 0; nX < m_aVirtualHosts.size(); nX++ )
            if ( hostMatches( m_aVirtualHosts[ nX ].first, strHost ) )
                return CString( m_aVirtualHosts[ nX ].first.c_str() );
        return CString( "" );
    }

        // The factories that serve a request: its virtual host's, or (for
        // other hosts, and with no request) the server's own.
    std::vector<CHttpHandlerFactory *> CHttpServer::factoriesFor( CHttpRequest *request )
    {
        if ( request != NULL && ! m_aVirtualHosts.empty() )
        {
            CString strHost = findVirtualHost( request->getHostName() );
            if ( strHost.GetLength() > 0 )
            {
                for ( size_t nX = 0; nX < m_aVirtualHosts.size(); nX++ )
                    if ( m_aVirtualHosts[ nX ].first == (const char *)strHost )
                        return m_aVirtualHosts[ nX ].second;
            }
        }
        std::vector<CHttpHandlerFactory *> factories;
        for ( size_t nX = 0; nX < m_aHandlerFactories.GetCount(); nX++ )
            factories.push_back( (CHttpHandlerFactory *)m_aHandlerFactories[ nX ] );
        return factories;
    }

    void *CHttpServer::Run( void )
    {
            // Intentionally empty: concrete servers accept the connections.
        return NULL;
    }

    CHttpHandler *CHttpServer::GetHandler( CHttpRequest *request, CUUID erid )
    {
        return GetHandler( request->getRequestType(), request, erid );
    }

    CHttpHandler *CHttpServer::GetHandler( const CString &method, CHttpRequest *request, CUUID erid )
    {
        std::vector<CHttpHandlerFactory *> factories = factoriesFor( request );
        for ( size_t nX = 0; nX < factories.size(); nX++ )
        {
            CHttpHandlerFactory *pFactory = factories[ nX ];
            CHttpHandler *pRetVal = pFactory->getHandler( method, request, erid );
            if ( pRetVal )
            {
                pRetVal->setServer( this );
                return pRetVal;
            }
        }
        return NULL;
    }

    void CHttpServer::AddHandlerFactory( CHttpHandlerFactory *pFactory )
    {
        if ( pFactory )
        {
            m_aHandlerFactories.Prepend( pFactory );
        }
    }

    void CHttpServer::RemoveHandlerFactory( CHttpHandlerFactory *pFactory )
    {
        for ( size_t nX = 0; nX < m_aHandlerFactories.GetCount(); nX++ )
        {
            if ( m_aHandlerFactories[ nX ] == pFactory )
            {
                m_aHandlerFactories.Remove( nX );
                break;
            }
        }
    }

    CString CHttpServer::getAllowedList( void )
    {
        return allowedFrom( factoriesFor( NULL ) );
    }

    CString CHttpServer::getAllowedList( CHttpRequest *request )
    {
        return allowedFrom( factoriesFor( request ) );
    }

    CString CHttpServer::allowedFrom( const std::vector<CHttpHandlerFactory *> &factories )
    {
        CStringArray all;
        bool bHasGet = false;
        bool bHasHead = false;

        for ( size_t nX = 0; nX < factories.size(); nX++ )
        {
            CStringArray methods = factories[ nX ]->getMethods();
            for ( size_t nY = 0; nY < methods.Count(); nY++ )
            {
                bool bFound = false;
                for ( size_t nZ = 0; nZ < all.Count(); nZ++ )
                {
                    if ( all[ nZ ] == methods[ nY ] )
                    {
                        bFound = true;
                        break;
                    }
                }
                if ( ! bFound )
                {
                    all.Append( methods[ nY ] );
                    if ( methods[ nY ] == "GET" ) bHasGet = true;
                    if ( methods[ nY ] == "HEAD" ) bHasHead = true;
                }
            }
        }

            // HEAD is always available where GET is.
        if ( bHasGet && ! bHasHead )
        {
            all.Append( "HEAD" );
        }

        CString retVal;
        for ( size_t nX = 0; nX < all.Count(); nX++ )
        {
            if ( nX > 0 )
                retVal += ", ";
            retVal += all[ nX ];
        }
        return retVal;
    }

    void CHttpServer::addStandardHeaders( CHttpResponse *response, bool keepAlive, bool http10 )
    {
            // RFC 9110 6.6.1: an origin server with a clock must send Date.
        response->addHeader( "Date", CHttpUtils::httpDate() );
        if ( m_strServerName.GetLength() > 0 && ! response->hasHeader( "Server" ) )
        {
            response->addHeader( "Server", m_strServerName );
        }
        if ( ! keepAlive )
        {
            response->addHeader( "Connection", "close" );
        }
        else if ( http10 )
        {
            response->addHeader( "Connection", "keep-alive" );
        }
    }

    CString CHttpServer::errorResponse( int nStatus )
    {
        CHttpResponse response;
        response.setStatus( nStatus );
        addStandardHeaders( &response, false, false );
        return response.toString();
    }

    CString CHttpServer::handleRequest( const CString &message, const CInternetAddress &source, bool &keepAlive )
    {
        return handleRequest( message, source, keepAlive, false, CString( "" ) );
    }

    CString CHttpServer::handleRequest( const CString &message, const CInternetAddress &source, bool &keepAlive,
                                        bool bSecure, const CString &strTlsServerName )
    {
        keepAlive = false;

        CHttpRequest request( source );
        CHttpResponse response;
        bool bParsed = false;
        request.setConnection( bSecure, strTlsServerName );

        try
        {
            bParsed = request.parse( message );
            processRequest( &request, bParsed, &response, keepAlive );
        }
        catch ( ... )
        {
            ERROR_LOG( "Unexpected exception while processing an HTTP request." );
            keepAlive = false;
            return errorResponse( 500 );
        }

        return response.toString();
    }

    void CHttpServer::processRequest( CHttpRequest *request, bool parseOk, CHttpResponse *response, bool &keepAlive )
    {
        keepAlive = false;
        bool bHttp10 = false;

        if ( ! parseOk )
        {
            response->setStatus( 400 );
            addStandardHeaders( response, false, false );
            return;
        }

            // HTTP-version = "HTTP/" DIGIT "." DIGIT (RFC 9112 2.3)
        CString version = request->getVersion();
        if ( version.GetLength() != 8 || version.Substring( 0, 5 ) != "HTTP/" || ! isdigit( (unsigned char)version[5] ) ||
             version[6] != '.' || ! isdigit( (unsigned char)version[7] ) )
        {
            response->setStatus( 400 );
            addStandardHeaders( response, false, false );
            return;
        }
        if ( version[5] != '1' )
        {
            response->setStatus( 505 );
            addStandardHeaders( response, false, false );
            return;
        }
        bHttp10 = ( version[7] == '0' );

            // Connection persistence (RFC 9112 9.3).
        CString connection;
        CStringArray connectionValues = request->getHeaderValues( "Connection" );
        for ( size_t nX = 0; nX < connectionValues.Count(); nX++ )
        {
            connection += ",";
            connection += connectionValues[ nX ];
        }
        connection.ToLowerCase();
        connection.Remove( ' ' );
        connection.Remove( '\t' );
        connection += ",";
        bool bClose = ( connection.IndexOf( ",close," ) != NOT_FOUND );
        bool bKeepAliveToken = ( connection.IndexOf( ",keep-alive," ) != NOT_FOUND );
        keepAlive = m_bUseKeepalive && ! bClose && ( ! bHttp10 || bKeepAliveToken );

            // RFC 9112 3.2: HTTP/1.1 requests need exactly one Host.
        CStringArray hosts = request->getHeaderValues( "Host" );
        if ( ( ! bHttp10 && hosts.Count() != 1 ) || hosts.Count() > 1 )
        {
            response->setStatus( 400 );
            keepAlive = false;
            addStandardHeaders( response, false, bHttp10 );
            return;
        }

            // RFC 9110 15.5.20: over TLS the request must be for the host the
            // client asked for in the handshake.
        if ( m_bRequireMatchingHost && request->isSecureConnection() )
        {
            CString strAskedFor = CHttpRequest::normalizeHostName( request->getTlsServerName() );
            CString strHost = request->getHostName();
            if ( strAskedFor.GetLength() > 0 && strHost.GetLength() > 0 && strHost != strAskedFor )
            {
                response->setStatus( 421 );
                keepAlive = false;
                addStandardHeaders( response, false, bHttp10 );
                return;
            }
        }

        CString expect = request->getHeaderValue( "Expect" );
        expect.Trim();
        expect.ToLowerCase();
        if ( expect.GetLength() > 0 && expect != "100-continue" )
        {
            response->setStatus( 417 );
            addStandardHeaders( response, keepAlive, bHttp10 );
            return;
        }

        CString method = request->getRequestType();
        bool bHead = ( method == "HEAD" );
        response->setHeadResponse( bHead );

        CUUID erid;

            // "OPTIONS *" asks about the server itself.
        if ( method == "OPTIONS" && request->getUri() == "*" )
        {
            response->setStatus( 200 );
            response->addHeader( "Allow", getAllowedList( request ) );
            addStandardHeaders( response, keepAlive, bHttp10 );
            return;
        }

        CHttpHandler *handler = GetHandler( method, request, erid );
        if ( handler == NULL && bHead )
        {
            handler = GetHandler( CString( "GET" ), request, erid );
        }

        if ( handler == NULL )
        {
            if ( CHttpUtils::isKnownMethod( method ) )
            {
                response->setStatus( 405 );
                response->addHeader( "Allow", getAllowedList( request ) );
            }
            else
            {
                response->setStatus( 501 );
            }
        }
        else
        {
            response->setStatus( 500 );
            bool bProcessed = false;
            try
            {
                bProcessed = handler->process( request, response );
            }
            catch ( ... )
            {
                ERROR_LOG( "Exception thrown by the %.32s handler.", (const char *)method );
                bProcessed = false;
            }
            int nStatus = response->getStatusCode();
            if ( ! bProcessed || nStatus < 100 || nStatus > 599 )
            {
                response->setStatus( 500 );
            }
            delete handler;
        }

            // A handler may ask for the connection to be closed.
        CString responseConnection = response->getHeaderValue( "Connection" );
        responseConnection.ToLowerCase();
        if ( responseConnection.IndexOf( "close" ) != NOT_FOUND )
        {
            keepAlive = false;
        }
        response->removeHeader( "Connection" );

        addStandardHeaders( response, keepAlive, bHttp10 );

        {
            CInternetAddress from( request->getInternetAddress() );
            INFO_LOG( "f=%.16s cip=%s erid=%s uri=\"%.256s\" sc=%d",
                      (const char *)method,
                      (const char *)from.toStringWithPort(),
                      (const char *)erid.toString(),
                      (const char *)request->getUri(),
                      response->getStatusCode() );
        }
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
