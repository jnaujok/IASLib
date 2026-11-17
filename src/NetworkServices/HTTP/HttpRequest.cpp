/**
 *  HTTP Request Class
 *
 *      This class provides an interface for generating an HTTP Request to
 * a remote server, or an incoming request to a server from a remote client.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: September 17, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "HttpRequest.h"

#include <cctype>
#include <string>

namespace IASLib
{
    const char *CHttpRequest::METHOD_GET = "GET";
    const char *CHttpRequest::METHOD_PUT = "PUT";
    const char *CHttpRequest::METHOD_HEAD = "HEAD";
    const char *CHttpRequest::METHOD_POST = "POST";
    const char *CHttpRequest::METHOD_OPTIONS = "OPTIONS";
    const char *CHttpRequest::METHOD_TRACE = "TRACE";
    const char *CHttpRequest::METHOD_DELETE = "DELETE";

    IMPLEMENT_OBJECT( CHttpRequest, CGenericRequest )

    CHttpRequest::CHttpRequest( const CInternetAddress &internetAddress ) : CGenericRequest( internetAddress ), m_bSecureConnection( false )
    {
    }

    CHttpRequest::CHttpRequest( const char *strMethod, const char *uri ) : CGenericRequest(), m_bSecureConnection( false )
    {
        m_requestType = strMethod;
        m_uri = uri;
        m_version = "HTTP/1.1";
    }

    CHttpRequest::~CHttpRequest( void )
    {
    }

    CString CHttpRequest::getPath( void )
    {
        CString target = m_uri;

            // absolute-form: scheme "://" authority path
        size_t nScheme = target.IndexOf( "://" );
        if ( nScheme != NOT_FOUND && target.IndexOf( '/' ) > nScheme )
        {
            size_t nPath = target.IndexOf( '/', nScheme + 3 );
            target = ( nPath == NOT_FOUND ) ? CString( "/" ) : target.Substring( nPath );
        }

        size_t nQuery = target.IndexOf( '?' );
        if ( nQuery != NOT_FOUND )
        {
            target = target.Substring( 0, (int)nQuery );
        }
        return target;
    }

    CString CHttpRequest::normalizeHostName( const CString &strHost )
    {
        std::string strName( (const char *)strHost );
        size_t nStart = strName.find_first_not_of( " \t" );
        size_t nEnd = strName.find_last_not_of( " \t" );
        strName = ( nStart == std::string::npos ) ? std::string() : strName.substr( nStart, nEnd - nStart + 1 );

        if ( ! strName.empty() && strName[0] == '[' )
        {
                // An IPv6 literal: "[::1]" or "[::1]:443".
            size_t nClose = strName.find( ']' );
            strName = ( nClose == std::string::npos ) ? std::string() : strName.substr( 0, nClose + 1 );
        }
        else
        {
            size_t nColon = strName.find( ':' );
            if ( nColon != std::string::npos )
                strName.erase( nColon );
        }
        while ( ! strName.empty() && strName[ strName.size() - 1 ] == '.' )
            strName.erase( strName.size() - 1 );
        for ( size_t n = 0; n < strName.size(); n++ )
            strName[ n ] = (char)tolower( (unsigned char)strName[ n ] );
        return CString( strName.c_str() );
    }

    CString CHttpRequest::getHostName( void )
    {
            // absolute-form targets carry the authority, which wins over Host.
        std::string strTarget( (const char *)m_uri );
        size_t nScheme = strTarget.find( "://" );
        if ( nScheme != std::string::npos && strTarget.find( '/' ) > nScheme )
        {
            size_t nStart = nScheme + 3;
            size_t nEnd = strTarget.find_first_of( "/?#", nStart );
            std::string strAuthority = strTarget.substr( nStart, ( nEnd == std::string::npos ) ? std::string::npos : nEnd - nStart );
            size_t nAt = strAuthority.rfind( '@' );
            if ( nAt != std::string::npos )
                strAuthority.erase( 0, nAt + 1 );
            return normalizeHostName( CString( strAuthority.c_str() ) );
        }
        return normalizeHostName( getHeaderValue( "Host" ) );
    }

    CString CHttpRequest::getQuery( void )
    {
        size_t nQuery = m_uri.IndexOf( '?' );
        if ( nQuery == NOT_FOUND )
        {
            return CString( "" );
        }
        CString query = m_uri.Substring( nQuery + 1 );
        size_t nFragment = query.IndexOf( '#' );
        if ( nFragment != NOT_FOUND )
        {
            query = query.Substring( 0, (int)nFragment );
        }
        return query;
    }

    CString CHttpRequest::toString( void )
    {
        CString retVal = m_requestType;
        retVal += " ";
        retVal += m_uri;
        retVal += " ";
        retVal += m_version;
        retVal += "\r\n";

        CString body;
        if ( m_bodyEntity )
        {
            body = m_bodyEntity->toString();
        }

        if ( m_pHeaders )
        {
            retVal += m_pHeaders->toString();
        }
        if ( body.GetLength() > 0 && ! hasHeader( "Content-Length" ) && ! hasHeader( "Transfer-Encoding" ) )
        {
            retVal += CString::FormatString( "Content-Length: %lu\r\n", (unsigned long)body.GetLength() );
        }
        retVal += "\r\n";
        retVal += body;
        return retVal;
    }

    void CHttpRequest::toStream( CStream *pStream )
    {
        CString message = toString();
        pStream->PutBuffer( (const char *)message, (int)message.GetLength() );
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
