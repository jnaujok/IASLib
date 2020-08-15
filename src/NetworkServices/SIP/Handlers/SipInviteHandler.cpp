/**
 * SIP Invite Handler class
 *
 * This class handles Invite requests for the SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 23, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "Logging/LogSink.h"
#include "NetworkServices/SIP/Handlers/SipInviteHandler.h"
#include "NetworkServices/Entities/NullEntity.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipInviteHandler, CSipHandler );

    IASLIB_REGISTER_SIP_HANDLER( CSipInviteHandler );

    CSipInviteHandler::CSipInviteHandler( CString uri, CString version ) : CSipHandler( uri, version )
    {

    }

    CSipInviteHandler::~CSipInviteHandler( void )
    {

    }

    bool CSipInviteHandler::process( CSipRequest *request, CSipResponse *response )
    {
        INFO_LOG( (const char *)request->toString() );

        response->addHeader( "From", request->getHeaderValue( "From" ) );
        response->addHeader( "To", request->getHeaderValue( "To" ) );
        response->addHeader( "Call-ID", request->getHeaderValue( "Call-ID" ) );
        response->addHeader( "CSeq", request->getHeaderValue( "CSeq" ) );
        response->addHeaders( "Via", request->getHeaderValues( "Via" ) );

        // This is not a number we manage
        response->setStatus( 404, "Not Found" );

        response->SetEntity( new CNullEntity() );

        return true;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
