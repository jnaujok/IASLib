/**
 * SIP Cancel Handler class
 *
 * This class handles Cancel requests for the SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "Logging/LogSink.h"
#include "NetworkServices/SIP/Handlers/SipCancelHandler.h"
#include "NetworkServices/Entities/NullEntity.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipCancelHandler, CSipHandler );
    IASLIB_REGISTER_SIP_HANDLER( CSipCancelHandler );


    CSipCancelHandler::CSipCancelHandler( CString uri, CString version ) : CSipHandler( uri, version )
    {

    }

    CSipCancelHandler::~CSipCancelHandler( void )
    {

    }

    bool CSipCancelHandler::process( CSipRequest *request, CSipResponse *response )
    {
        INFO_LOG( (const char *)request->toString() );

        response->addHeader( "From", request->getHeaderValue( "From" ) );
        response->addHeader( "To", request->getHeaderValue( "To" ) );
        response->addHeader( "Call-ID", request->getHeaderValue( "Call-ID" ) );
        response->addHeader( "CSeq", request->getHeaderValue( "CSeq" ) );
        response->addHeaders( "Via", request->getHeaderValues( "Via" ) );

        // Cancel returns a 200 to acknowledge the reception of a CANCEL. 
        // If we had an active session, we would then cancel it, which
        // would trigger the 487 (request terminated) response.
        response->setStatus( 200, "OK" );

        CEntity *entity = new CNullEntity();

        // Set entity
        response->SetEntity( entity );

        return true;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__

