/**
 * SIP Bye Handler class
 *
 * This class handles Bye requests for the SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "Logging/LogSink.h"
#include "NetworkServices/SIP/Handlers/SipByeHandler.h"
#include "NetworkServices/Entities/NullEntity.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipByeHandler, CSipHandler );
    IASLIB_REGISTER_SIP_HANDLER( CSipByeHandler );


    CSipByeHandler::CSipByeHandler( CString uri, CString version ) : CSipHandler( uri, version )
    {

    }

    CSipByeHandler::~CSipByeHandler( void )
    {

    }

    bool CSipByeHandler::process( CSipRequest *request, CSipResponse *response )
    {
        INFO_LOG( (const char *)request->toString() );

        response->addHeader( "From", request->getHeaderValue( "From" ) );
        response->addHeader( "To", request->getHeaderValue( "To" ) );
        response->addHeader( "Call-ID", request->getHeaderValue( "Call-ID" ) );
        response->addHeader( "CSeq", request->getHeaderValue( "CSeq" ) );
        response->addHeaders( "Via", request->getHeaderValues( "Via" ) );

        // The BYE should be acknowledged with a 200, and end the session.
        response->setStatus( 200, "OK" );

        CEntity *entity = new CNullEntity();

        // Set entity
        response->SetEntity( entity );

        return true;
    }
};

#endif // IASLIB_NETWORKING__
