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
#include "NetworkServices/SIP/SipUtils.h"

namespace IASLib
{
    IMPLEMENT_OBJECT(CSipResponse, CGenericResponse );

    CSipResponse::CSipResponse( void ) : CGenericResponse()
    {
        m_nStatusCode = 0;
        m_strStatusDescription = "Unknown";
    }

    CSipResponse::CSipResponse( CStream &responseStream ) : CGenericResponse( responseStream )
    {
        m_nStatusCode = 0;
        m_strStatusDescription = "Unknown";
    }

    CSipResponse::~CSipResponse( void )
    {
        // The base class deletes the body.
    }

    void CSipResponse::setStatus( int nCode )
    {
        CGenericResponse::setStatus( nCode, CSipUtils::reasonPhrase( nCode ) );
    }

    void CSipResponse::toStream( CStream *pStream )
    {
        CString message = toString();
        pStream->PutBuffer( (const char *)message, (int)message.GetLength() );
    }

    CString CSipResponse::toString( void )
    {
        CString statusLine = "SIP/2.0 ";
        statusLine += CString::FormatString( "%d", (int)m_nStatusCode );
        statusLine += " ";
        statusLine += m_strStatusDescription;
        statusLine += "\r\n";
        return serialize( statusLine );
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
