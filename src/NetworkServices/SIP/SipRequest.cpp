/**
 * SIP Request class
 *
 * This class encapsulates a SIP based request, including the request line,
 * headers and body.
 *
 * Author: Jeffrey R. Naujok
 * Created: June 17, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/SipHeaderList.h"
#include "NetworkServices/SIP/SipRequest.h"
#include "NetworkServices/SIP/SipUtils.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipRequest, CGenericRequest );

    CSipRequest::CSipRequest( const CInternetAddress &internetAddress ) : CGenericRequest( internetAddress )
    {
    }

    CSipRequest::CSipRequest( void ) : CGenericRequest()
    {
        m_version = "SIP/2.0";
    }

    CSipRequest::~CSipRequest( void )
    {
    }

    CString CSipRequest::getTopVia( void )
    {
        CStringArray vias = getHeaderValues( "Via" );
        if ( vias.Count() == 0 )
        {
            return CString( "" );
        }

        CStringArray first = CSipUtils::splitList( vias[0] );
        if ( first.Count() == 0 )
        {
            return CString( "" );
        }
        return first[0];
    }

    void CSipRequest::setTopVia( const CString &via )
    {
        CStringArray vias = getHeaderValues( "Via" );
        if ( vias.Count() == 0 )
        {
            getHeaders()->addHeader( "Via", via );
            return;
        }

            // Rebuild the first Via header value with the new top element and
            // keep everything else as it was.
        CStringArray first = CSipUtils::splitList( vias[0] );
        CString rebuilt = via;
        for ( size_t nX = 1; nX < first.Count(); nX++ )
        {
            rebuilt += ", ";
            rebuilt += first[ nX ];
        }

        CStringArray updated;
        updated.Append( rebuilt );
        for ( size_t nX = 1; nX < vias.Count(); nX++ )
        {
            updated.Append( vias[ nX ] );
        }
        setHeaderValues( "Via", updated );
    }

    CString CSipRequest::getFromTag( void )
    {
        return CSipUtils::getTag( getHeaderValue( "From" ) );
    }

    CString CSipRequest::getToTag( void )
    {
        return CSipUtils::getTag( getHeaderValue( "To" ) );
    }

    CString CSipRequest::toString( void )
    {
        CString retVal = m_requestType;
        retVal += " ";
        retVal += m_uri;
        retVal += " ";
        retVal += m_version;
        retVal += "\r\n";

        CString body;
        if ( m_bodyEntity )
        {
            body = m_bodyEntity->toString();
        }

        if ( m_pHeaders )
        {
            retVal += m_pHeaders->toString();
        }
        if ( ! hasHeader( "Content-Length" ) )
        {
            retVal += CString::FormatString( "Content-Length: %lu\r\n", (unsigned long)body.GetLength() );
        }
        retVal += "\r\n";
        retVal += body;

        return retVal;
    }

    void CSipRequest::toStream( CStream *pStream )
    {
        CString message = toString();
        pStream->PutBuffer( (const char *)message, (int)message.GetLength() );
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
