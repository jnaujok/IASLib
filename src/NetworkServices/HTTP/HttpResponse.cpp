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
#include "Streams/StringStream.h"

#include <cctype>
#include <string>

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

    bool CHttpResponse::parse( const CString &message )
    {
        m_bInbound = true;
        m_headers.clear();
        SetEntity( NULL );
        m_strVersion = "";

        std::string text( (const char *)message, message.GetLength() );
        size_t nStart = 0;
        while ( nStart < text.size() && ( text[ nStart ] == '\r' || text[ nStart ] == '\n' ) )
            nStart++;
        size_t nLineEnd = text.find( '\n', nStart );
        if ( nLineEnd == std::string::npos )
        {
            return false;
        }
        std::string line = text.substr( nStart, nLineEnd - nStart );
        if ( ! line.empty() && line[ line.size() - 1 ] == '\r' )
            line.erase( line.size() - 1 );

            // HTTP-version SP 3DIGIT [ SP reason-phrase ]
        if ( line.size() < 12 || line.compare( 0, 5, "HTTP/" ) != 0 || line[8] != ' ' ||
             ! isdigit( (unsigned char)line[9] ) || ! isdigit( (unsigned char)line[10] ) || ! isdigit( (unsigned char)line[11] ) ||
             ( line.size() > 12 && line[12] != ' ' ) )
        {
            return false;
        }
        m_strVersion = CString( line.substr( 0, 8 ).c_str() );
        int nStatus = ( line[9] - '0' ) * 100 + ( line[10] - '0' ) * 10 + ( line[11] - '0' );
        CGenericResponse::setStatus( nStatus, ( line.size() > 13 ) ? line.substr( 13 ).c_str() : "" );

        CStringStream stream( text.data() + nLineEnd + 1, text.size() - nLineEnd - 1 );
        bool bComplete = m_headers.parse( stream );

        std::string body;
        char achBuffer[ 8192 ];
        while ( ! stream.IsEOS() )
        {
            int nRead = stream.GetBuffer( achBuffer, (int)sizeof( achBuffer ) );
            if ( nRead <= 0 )
                break;
            body.append( achBuffer, (size_t)nRead );
        }
        if ( ! body.empty() )
        {
            SetEntity( CEntity::getEntity( m_headers.firstValue( "Content-Type" ), CString::FromBytes( body.data(), body.size() ) ) );
        }
        return bComplete && ! m_headers.isMalformed();
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
