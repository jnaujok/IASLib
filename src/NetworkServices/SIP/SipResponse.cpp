/**
 * SIP Response class
 *
 * This class encapsulates a SIP based response.
 *
 * Author: Jeffrey R. Naujok
 * Created: July 21, 2019
 *
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/SipResponse.h"

namespace IASLib
{
    IMPLEMENT_OBJECT(CSipResponse, CGenericResponse );

    CSipResponse::CSipResponse( CStream &responseStream ) : CGenericResponse( responseStream )
    {
        m_nStatusCode = 0;
        m_strStatusDescription = "Unknown";
        m_body = NULL;
    }

    CSipResponse::~CSipResponse( void )
    {
        if ( m_body )
            delete m_body;
        m_body = NULL;
    }

 
    void CSipResponse::toStream( CStream *pStream )
    {
        CString statusLine;
        statusLine.Format( "SIP/2.0 %d %s\r\n", (int)m_nStatusCode, (const char *)m_strStatusDescription );
        pStream->PutLine( statusLine );

        pStream->PutLine( this->m_headers.toString() );
        pStream->PutLine( "\r\n" );
        if ( m_body )
        {
            pStream->PutLine( this->m_body->toString() );
        }
    }

    CString CSipResponse::toString( void )
    {
        CString strRetVal;
        CString statusLine;
        statusLine.Format( "SIP/2.0 %d %s\n", (int)m_nStatusCode, (const char *)m_strStatusDescription );
        strRetVal += statusLine;

        strRetVal += this->m_headers.toString();
        strRetVal += "\n";
        if ( m_body )
        {
            strRetVal += this->m_body->toString();
        }
        
        return strRetVal;
    }

   

} // namespace IASLib

#endif // IASLIB_NETWORKING__
