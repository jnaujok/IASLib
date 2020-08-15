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

    CGenericResponse::CGenericResponse( void ) : m_responseStream( GENERIC_STREAM )
    {
        m_nStatusCode = 500;
        m_strStatusDescription = "Internal Server Error";
        m_bInbound = false;
    }

    CGenericResponse::CGenericResponse( CStream &responseStream ) : m_responseStream( responseStream )
    {
        m_nStatusCode = 500;
        m_strStatusDescription = "Internal Server Error";
        m_bInbound = false;
    }

    CGenericResponse::CGenericResponse( CStream &inputStream, CInternetAddress &internetAddress ) : m_responseStream( inputStream )
    {
        m_nStatusCode = 500;
        m_strStatusDescription = "Internal Server Error";
        m_bInbound = true;
    }

    CGenericResponse::~CGenericResponse( void )
    {
        if ( m_body )
        {
            delete m_body;
        }
        m_body = NULL;
    }

    void CGenericResponse::setStatus( int nCode, const char *desc )
    {
        m_nStatusCode = nCode;
        m_strStatusDescription = desc;
    }

    void CGenericResponse::addHeader( const char *name, const char *value )
    {
        m_headers.addHeader( name, value );
    }

    void CGenericResponse::addHeader( const char *name, size_t nValue )
    {
        CString value;
        value.Format( "%d", nValue );
        m_headers.addHeader( name, value );
    }

    void CGenericResponse::addHeaders( const char *name, CStringArray values )
    {
        m_headers.addHeader( name, values );
    }
    
    CString CGenericResponse::getHeaderValue( const char *name )
    {
        if ( m_headers.hasHeader( name ) )
        {
            return m_headers.firstValue( name );
        }

    }

    CStringArray CGenericResponse::getHeaderValues( const char *name )
    {

    }

}; // namespace IASLib

#endif // IASLIB_NETWORKING__
