/**
 * Generic Response class
 *
 * This class encapsulates a Generic response.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 19, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/GenericResponse.h"
#include "Streams/NullStream.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CGenericResponse, CObject );

    static CNullStream GENERIC_STREAM;

    CGenericResponse::CGenericResponse( void )
    {
        m_nStatusCode = 500;
        m_strStatusDescription = "Internal Server Error";
        m_bInbound = false;
        m_body = NULL;
        m_pResponseStream = NULL;
    }

    CGenericResponse::CGenericResponse( CStream &responseStream )
    {
        m_nStatusCode = 500;
        m_strStatusDescription = "Internal Server Error";
        m_bInbound = false;
        m_body = NULL;
        m_pResponseStream = &responseStream;
    }

    CGenericResponse::CGenericResponse( CStream &inputStream, CInternetAddress &internetAddress )
    {
        m_nStatusCode = 500;
        m_strStatusDescription = "Internal Server Error";
        m_bInbound = true;
        m_body = NULL;
        m_pResponseStream = &inputStream;
    }

    CGenericResponse::~CGenericResponse( void )
    {
        if ( m_body )
        {
            delete m_body;
        }
        m_body = NULL;
    }

    CStream &CGenericResponse::getResponseStream( void )
    {
        if ( m_pResponseStream )
        {
            return *m_pResponseStream;
        }
        return GENERIC_STREAM;
    }

    void CGenericResponse::setStatus( int nCode, const char *desc )
    {
        m_nStatusCode = nCode;
            // (Any text was taken, so a reason with a CR LF in it could add
            // header lines to the response.)
        CString strDescription( desc ? desc : "" );
        m_strStatusDescription = CHeader::isValidValue( strDescription ) ? strDescription : CString( "" );
    }

    void CGenericResponse::SetEntity( CEntity *entity )
    {
        if ( ( m_body ) && ( m_body != entity ) )
        {
            delete m_body;
        }
        m_body = entity;
    }

    bool CGenericResponse::addHeader( const char *name, const char *value )
    {
        return m_headers.addHeader( name, value );
    }

    bool CGenericResponse::addHeader( const char *name, size_t nValue )
    {
        CString value;
        value.Format( "%lu", (unsigned long)nValue );
        return m_headers.addHeader( name, value );
    }

    bool CGenericResponse::addHeaders( const char *name, CStringArray values )
    {
        return m_headers.addHeader( name, values );
    }

    void CGenericResponse::removeHeader( const char *name )
    {
        m_headers.removeHeader( name );
    }

    bool CGenericResponse::hasHeader( const char *name )
    {
        return m_headers.hasHeader( name );
    }

    CString CGenericResponse::getHeaderValue( const char *name )
    {
        return m_headers.firstValue( name );
    }

    CStringArray CGenericResponse::getHeaderValues( const char *name )
    {
        return m_headers.allValues( name );
    }

    CString CGenericResponse::serialize( const CString &statusLine )
    {
        CString body;
        if ( m_body )
        {
            body = m_body->toString();
        }

        if ( ( body.GetLength() > 0 ) && ( ! m_headers.hasHeader( "Content-Type" ) ) && ( m_body ) )
        {
            m_headers.addHeader( "Content-Type", m_body->getMimeType() );
        }
            // A message with Transfer-Encoding is framed by it and must not
            // also carry Content-Length (RFC 9112 6.1, 6.2). (Content-Length
            // was always added, so a chunked response had both, which lets a
            // proxy and a client disagree on where the message ends.)
        if ( m_headers.hasHeader( "Transfer-Encoding" ) )
        {
            m_headers.removeHeader( "Content-Length" );
        }
        else
        {
            addHeader( "Content-Length", (size_t)body.GetLength() );
        }

        CString retVal = statusLine;
        retVal += m_headers.toString();
        retVal += "\r\n";
        retVal += body;
        return retVal;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
