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

    CHttpRequest::CHttpRequest( const CInternetAddress &internetAddress ) : CGenericRequest( internetAddress )
    {
    }

    CHttpRequest::CHttpRequest( const char *strMethod, const char *uri ) : CGenericRequest()
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
