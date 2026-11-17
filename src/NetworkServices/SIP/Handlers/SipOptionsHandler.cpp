/**
 * SIP Options Handler class
 *
 * Author: Jeffrey R. Naujok
 * Created: September 13, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/Handlers/SipOptionsHandler.h"
#include "NetworkServices/SIP/SipServer.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipOptionsHandler, CSipHandler );

    CSipOptionsHandler::CSipOptionsHandler( CString uri, CString version ) : CSipHandler( uri, version )
    {
    }

    CSipOptionsHandler::~CSipOptionsHandler( void )
    {
    }

    bool CSipOptionsHandler::process( CSipRequest *request, CSipResponse *response )
    {
            // Many systems use OPTIONS as a keep-alive, so answer 200 OK with
            // what this server can do.
        response->setStatus( 200 );

        if ( m_pServer )
        {
            response->addHeader( "Allow", m_pServer->getAllowedList() );

            CString supported = m_pServer->getSupportedList();
            if ( supported.GetLength() > 0 )
            {
                response->addHeader( "Supported", supported );
            }
        }

        response->addHeader( "Accept", "application/sdp" );
        response->addHeader( "Accept-Encoding", "identity" );
        response->addHeader( "Accept-Language", "en" );

        return true;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
