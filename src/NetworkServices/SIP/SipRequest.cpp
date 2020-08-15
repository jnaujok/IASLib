/**
 * SIP Request class
 *
 * This class encapsulates a SIP based request, including the request line,
 * headers, and a handle to the stream with the request.
 *
 * Author: Jeffrey R. Naujok
 * Created: June 17, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/SipHeaderList.h"
#include "NetworkServices/SIP/SipRequest.h"
#include "BaseTypes/StringTokenizer.h"
#include "Logging/LogSink.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipRequest, CGenericRequest );

    CSipRequest::CSipRequest( CInternetAddress &internetAddress ) : CGenericRequest( internetAddress )
    {

    }

    CSipRequest::~CSipRequest( void )
    {
    }

    CString CSipRequest::toString( void )
    {
        CString retVal;
        retVal.Format( "%s %s %s\r\n",  (const char *)m_requestType, (const char *)m_uri, (const char *)m_version );
        retVal += m_pHeaders->toString();
        retVal += "\r\n";
        retVal += m_bodyEntity->toString();

        return retVal;
    }

    void CSipRequest::toStream( CStream *pStream )
    {
        CString statusLine;
        statusLine.Format( "%s %s %s\r\n", (const char *)getRequestType(), (const char *)getUri(), (const char *)getVersion() );
        pStream->PutLine( statusLine );

        pStream->PutLine( m_pHeaders->toString() );
        pStream->PutLine( "\r\n" );
        if ( m_bodyEntity )
        {
            pStream->PutLine( m_bodyEntity->toString() );
        }
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__