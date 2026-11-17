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
        m_strStatusDescription = desc;
    }

    void CGenericResponse::SetEntity( CEntity *entity )
    {
        if ( ( m_body ) && ( m_body != entity ) )
        {
            delete m_body;
        }
        m_body = entity;
    }

    void CGenericResponse::addHeader( const char *name, const char *value )
    {
        m_headers.addHeader( name, value );
    }

    void CGenericResponse::addHeader( const char *name, size_t nValue )
    {
        CString value;
        value.Format( "%lu", (unsigned long)nValue );
        m_headers.addHeader( name, value );
    }

    void CGenericResponse::addHeaders( const char *name, CStringArray values )
    {
        m_headers.addHeader( name, values );
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
        addHeader( "Content-Length", (size_t)body.GetLength() );

        CString retVal = statusLine;
        retVal += m_headers.toString();
        retVal += "\r\n";
        retVal += body;
        return retVal;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
