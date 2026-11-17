/**
 *  HTTP Response Class
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 9/07/2006
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/HttpResponse.h"
#include "NetworkServices/HTTP/HttpUtils.h"

namespace IASLib
{
    IMPLEMENT_OBJECT(CHttpResponse, CGenericResponse );

    CHttpResponse::CHttpResponse( void ) : CGenericResponse(), m_remoteAddress()
    {
        m_bHeadResponse = false;
    }

    CHttpResponse::CHttpResponse( CStream &responseStream, CInternetAddress &internetAddress ) : CGenericResponse( responseStream ), m_remoteAddress( internetAddress )
    {
        m_bHeadResponse = false;
    }

    CHttpResponse::~CHttpResponse( void )
    {
        // The base class deletes the body.
    }

    void CHttpResponse::setStatus( int nCode )
    {
        CGenericResponse::setStatus( nCode, CHttpUtils::reasonPhrase( nCode ) );
    }

    void CHttpResponse::toStream( CStream *pStream )
    {
        CString message = toString();
        pStream->PutBuffer( (const char *)message, (int)message.GetLength() );
    }

    CString CHttpResponse::toString( void )
    {
        CString statusLine = "HTTP/1.1 ";
        statusLine += CString::FormatString( "%d", (int)m_nStatusCode );
        statusLine += " ";
        statusLine += m_strStatusDescription;
        statusLine += "\r\n";

            // RFC 9110 6.4.1 / 8.6: 1xx and 204 responses have no body and no
            // Content-Length; a 304 has no body.
        if ( ( m_nStatusCode >= 100 && m_nStatusCode < 200 ) || m_nStatusCode == 204 || m_nStatusCode == 304 )
        {
            if ( m_nStatusCode != 304 )
            {
                m_headers.removeHeader( "Content-Length" );
            }
            CString retVal = statusLine;
            retVal += m_headers.toString();
            retVal += "\r\n";
            return retVal;
        }

        CString message = serialize( statusLine );

        if ( m_bHeadResponse )
        {
            size_t nEnd = message.IndexOf( "\r\n\r\n" );
            if ( nEnd != NOT_FOUND )
            {
                message = message.Substring( 0, (int)( nEnd + 4 ) );
            }
        }
        return message;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
