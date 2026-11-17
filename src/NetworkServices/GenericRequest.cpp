/**
 * Generic Request class
 *
 * This class encapsulates a Generic request, including the request line,
 * headers and body.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 19, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/GenericRequest.h"
#include "Streams/StringStream.h"
#include "Logging/LogSink.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CGenericRequest, CObject );

    CGenericRequest::CGenericRequest( const CInternetAddress &internetAddress ) : m_InternetAddress( internetAddress )
    {
        m_bIsValid = false;
        m_bInbound = true;
        m_pHeaders = NULL;
        m_bodyEntity = NULL;
    }

    CGenericRequest::CGenericRequest( void )
    {
        m_bIsValid = true;
        m_bInbound = false;
        m_pHeaders = NULL;
        m_bodyEntity = NULL;
    }

    CGenericRequest::~CGenericRequest( void )
    {
        if ( m_pHeaders )
            delete m_pHeaders;
        m_pHeaders = NULL;
        if ( m_bodyEntity )
            delete m_bodyEntity;
        m_bodyEntity = NULL;
    }

    CString CGenericRequest::stripLineEnding( const CString &line )
    {
        if ( ( line.GetLength() > 0 ) && ( line[ line.GetLength() - 1 ] == '\r' ) )
        {
            return line.Substring( 0, (int)line.GetLength() - 1 );
        }
        return line;
    }

    void CGenericRequest::setParseError( const char *reason )
    {
        if ( m_strParseError.GetLength() == 0 )
        {
            m_strParseError = reason;
        }
        m_bIsValid = false;
    }

    bool CGenericRequest::parse( const CString &message )
    {
        CStringStream stream( message );
        return parse( stream );
    }

    bool CGenericRequest::parse( CStream &requestStream )
    {
        m_bIsValid = false;
        m_bInbound = true;
        m_strParseError = "";

        if ( m_pHeaders )
        {
            delete m_pHeaders;
            m_pHeaders = NULL;
        }
        if ( m_bodyEntity )
        {
            delete m_bodyEntity;
            m_bodyEntity = NULL;
        }

            // Skip any blank lines before the request line (RFC 3261 7.5,
            // RFC 7230 3.5). These include keep-alive CRLFs.
        CString line;
        bool bFound = false;
        while ( ! requestStream.IsEOS() )
        {
            line = stripLineEnding( requestStream.GetLine() );
            if ( line.GetLength() > 0 )
            {
                bFound = true;
                break;
            }
        }

        if ( ! bFound )
        {
            setParseError( "Empty message" );
            return false;
        }

            // Request-Line = Method SP Request-URI SP Version CRLF
        size_t nFirst = line.IndexOf( ' ' );
        size_t nLast = line.LastIndexOf( ' ' );
        if ( ( nFirst == NOT_FOUND ) || ( nFirst == nLast ) )
        {
            setParseError( "Malformed request line" );
            return false;
        }

        m_requestType = line.Substring( 0, (int)nFirst );
        m_uri = line.Substring( nFirst + 1, (int)( nLast - nFirst - 1 ) );
        m_version = line.Substring( nLast + 1 );
        m_uri.Trim();

        if ( ( m_requestType.GetLength() == 0 ) || ( m_uri.GetLength() == 0 ) || ( m_version.GetLength() == 0 ) ||
             ( m_uri.IndexOf( ' ' ) != NOT_FOUND ) )
        {
            setParseError( "Malformed request line" );
            return false;
        }

            // Use the protocol's header list type.
        m_pHeaders = this->getHeaderList( requestStream );

        bool bOk = true;
        if ( m_pHeaders->isMalformed() )
        {
            setParseError( "Malformed header line" );
            bOk = false;
        }

        size_t nAvailable = requestStream.bytesRemaining();
        size_t nContentLength = nAvailable;

        if ( m_pHeaders->hasHeader( "content-length" ) )
        {
            CString contentLength = m_pHeaders->firstValue( "content-length" );
            contentLength.Trim();
            if ( ( contentLength.GetLength() == 0 ) || ( ! contentLength.IsDigits() ) || ( contentLength.GetLength() > 9 ) )
            {
                setParseError( "Invalid Content-Length" );
                bOk = false;
                nContentLength = 0;
            }
            else
            {
                nContentLength = (size_t)atol( contentLength );
                if ( nContentLength > nAvailable )
                {
                    setParseError( "Message body is shorter than Content-Length" );
                    bOk = false;
                    nContentLength = nAvailable;
                }
                    // Any bytes beyond Content-Length are ignored (RFC 3261 18.3).
            }
        }

        CString contentType = m_pHeaders->firstValue( "content-type" );
        m_bodyEntity = CEntity::getEntity( contentType, requestStream, nContentLength );

        m_bIsValid = bOk;
        return m_bIsValid;
    }

    void CGenericRequest::setRequestType( const char *requestType )
    {
        m_requestType = requestType;
    }

    void CGenericRequest::setUri( const char *uri )
    {
        m_uri = uri;
    }

    void CGenericRequest::setVersion( const char *version )
    {
        m_version = version;
    }

    CHeaderList *CGenericRequest::getHeaders( void )
    {
        if ( m_pHeaders == NULL )
        {
            m_pHeaders = newHeaderList();
        }
        return m_pHeaders;
    }

    CString CGenericRequest::getHeaderValue( const char *headerName )
    {
        if ( m_pHeaders )
        {
            return m_pHeaders->firstValue( headerName );
        }
        return CString( "" );
    }

    void CGenericRequest::setHeaderValue( const char *headerName, const char *headerValue )
    {
        getHeaders()->addHeader( headerName, headerValue );
    }

    CStringArray CGenericRequest::getHeaderValues( const char *headerName )
    {
        if ( m_pHeaders )
        {
            return m_pHeaders->allValues( headerName );
        }
        return CStringArray();
    }

    void CGenericRequest::setHeaderValues( const char *headerName, CStringArray headerValues )
    {
        getHeaders()->removeHeader( headerName );
        getHeaders()->addHeader( headerName, headerValues );
    }

    bool CGenericRequest::hasHeader( const char *headerName )
    {
        return ( m_pHeaders != NULL ) && m_pHeaders->hasHeader( headerName );
    }

    void CGenericRequest::removeHeader( const char *headerName )
    {
        if ( m_pHeaders )
        {
            m_pHeaders->removeHeader( headerName );
        }
    }

    void CGenericRequest::setEntity( CEntity *entity )
    {
        if ( ( m_bodyEntity ) && ( m_bodyEntity != entity ) )
        {
            delete m_bodyEntity;
        }
        m_bodyEntity = entity;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
