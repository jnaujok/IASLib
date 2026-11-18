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

#include <string>

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

            // (Only spaces in the URI were refused, so a URI holding a bare CR
            // or other control characters was accepted and could be echoed.)
        if ( ! isValidMethod( m_requestType ) || ! isValidUri( m_uri ) || ! isValidVersion( m_version ) )
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

            // The header section must end with its blank line; without it the
            // message was cut short, and whatever followed is not a body.
            // (A truncated message was accepted with the headers it had.)
        if ( ! m_pHeaders->isComplete() )
        {
            setParseError( "Incomplete header section" );
            m_bodyEntity = CEntity::getEntity( "", CString( "" ) );
            return false;
        }

            // How much of the stream is left, which is the body when there is
            // no Content-Length. Streams that cannot tell (one that does not
            // override bytesRemaining()) return NOT_FOUND, and the body is
            // then everything up to the end of the stream. (NOT_FOUND was
            // taken as the body length, so such a message asked for a buffer
            // of (size_t)-1 + 1 bytes.)
        size_t nAvailable = requestStream.bytesRemaining();
        bool bHasLength = false;
        size_t nContentLength = 0;

        if ( m_pHeaders->hasHeader( "content-length" ) )
        {
            CString contentLength = m_pHeaders->firstValue( "content-length" );
            contentLength.Trim();
            if ( ( contentLength.GetLength() == 0 ) || ( ! contentLength.IsDigits() ) || ( contentLength.GetLength() > 9 ) )
            {
                setParseError( "Invalid Content-Length" );
                bOk = false;
            }
            else
            {
                bHasLength = true;
                nContentLength = (size_t)atol( contentLength );
                    // Any bytes beyond Content-Length are ignored (RFC 3261 18.3).
            }
        }

            // The body: Content-Length bytes, or (without one) all that remain.
            // It is read in pieces until it is complete or the stream ends, so
            // a stream that delivers it in parts (a socket) gives all of it,
            // and a large Content-Length on a short message allocates only
            // what arrives. (It was one read of a buffer of Content-Length
            // bytes, checked against bytesRemaining(), which a socket stream
            // reports as 0, so every body there looked too short.)
        size_t nWanted = bHasLength ? nContentLength : nAvailable;
        std::string body;
        readBody( requestStream, body, nWanted );

        if ( bHasLength && body.size() < nContentLength )
        {
            setParseError( "Message body is shorter than Content-Length" );
            bOk = false;
        }

        CString contentType = m_pHeaders->firstValue( "content-type" );
        m_bodyEntity = CEntity::getEntity( contentType, CString::FromBytes( body.data(), body.size() ) );

        m_bIsValid = bOk;
        return m_bIsValid;
    }

    void CGenericRequest::readBody( CStream &stream, std::string &body, size_t nWanted )
    {
        char achBuffer[ 8192 ];
        while ( body.size() < nWanted && ! stream.IsEOS() )
        {
            size_t nChunk = nWanted - body.size();
            if ( nChunk > sizeof( achBuffer ) )
            {
                nChunk = sizeof( achBuffer );
            }
            int nRead = stream.GetBuffer( achBuffer, (int)nChunk );
            if ( nRead <= 0 )
            {
                break;
            }
            body.append( achBuffer, (size_t)nRead );
        }
    }

    bool CGenericRequest::isValidMethod( const CString &method )
    {
        return CHeader::isValidName( method );
    }

    bool CGenericRequest::isValidUri( const CString &uri )
    {
        const char *pText = (const char *)uri;
        size_t nLength = uri.GetLength();
        if ( nLength == 0 )
        {
            return false;
        }
        for ( size_t nX = 0; nX < nLength; nX++ )
        {
            unsigned char uch = (unsigned char)pText[ nX ];
            if ( uch <= ' ' || uch == 0x7F )
            {
                return false;
            }
        }
        return true;
    }

    bool CGenericRequest::setRequestType( const char *requestType )
    {
        CString strValue( requestType ? requestType : "" );
        if ( ! isValidMethod( strValue ) )
        {
            return false;
        }
        m_requestType = strValue;
        return true;
    }

    bool CGenericRequest::setUri( const char *uri )
    {
            // (Any text was taken, so a URI built from user input could carry
            // a CR LF and inject header lines into the request.)
        CString strValue( uri ? uri : "" );
        if ( ! isValidUri( strValue ) )
        {
            return false;
        }
        m_uri = strValue;
        return true;
    }

    bool CGenericRequest::setVersion( const char *version )
    {
        CString strValue( version ? version : "" );
        if ( ! isValidVersion( strValue ) )
        {
            return false;
        }
        m_version = strValue;
        return true;
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

    bool CGenericRequest::setHeaderValue( const char *headerName, const char *headerValue )
    {
        return getHeaders()->addHeader( headerName, headerValue );
    }

    CStringArray CGenericRequest::getHeaderValues( const char *headerName )
    {
        if ( m_pHeaders )
        {
            return m_pHeaders->allValues( headerName );
        }
        return CStringArray();
    }

    bool CGenericRequest::setHeaderValues( const char *headerName, CStringArray headerValues )
    {
        CString name( headerName ? headerName : "" );
        name.Trim();
        if ( ! CHeader::isValidName( name ) )
        {
            return false;
        }
        for ( size_t nX = 0; nX < headerValues.Count(); nX++ )
        {
            if ( ! CHeader::isValidValue( headerValues[ nX ] ) )
            {
                return false;
            }
        }
        getHeaders()->removeHeader( headerName );
        return getHeaders()->addHeader( headerName, headerValues );
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
