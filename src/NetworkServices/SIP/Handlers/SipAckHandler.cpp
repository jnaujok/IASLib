/**
 * SIP Ack Handler class
 *
 * This class handles Ack requests for the SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "Logging/LogSink.h"
#include "NetworkServices/SIP/Handlers/SipAckHandler.h"
#include "NetworkServices/Entities/NullEntity.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipAckHandler, CSipHandler );
    IASLIB_REGISTER_SIP_HANDLER( CSipAckHandler );

    CSipAckHandler::CSipAckHandler( CString uri, CString version ) : CSipHandler( uri, version )
    {

    }

    CSipAckHandler::~CSipAckHandler( void )
    {

    }

    bool CSipAckHandler::process( CSipRequest *request, CSipResponse *response )
    {
        INFO_LOG( (const char *)request->toString() );

        response->addHeader( "From", request->getHeaderValue( "From" ) );
        response->addHeader( "To", request->getHeaderValue( "To" ) );
        response->addHeader( "Call-ID", request->getHeaderValue( "Call-ID" ) );
        response->addHeader( "CSeq", request->getHeaderValue( "CSeq" ) );
        response->addHeaders( "Via", request->getHeaderValues( "Via" ) );

        // The ACK can be acknowledged with a 200, but it's optional. Since
        // we designed around request/response, we go ahead and respond with
        // a 200.
        response->setStatus( 200, "OK" );

        CEntity *entity = new CNullEntity();

        // Set entity
        response->SetEntity( entity );

        return true;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__

