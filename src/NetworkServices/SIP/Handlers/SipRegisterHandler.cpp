/**
 * SIP Register Handler class
 *
 * This class handles Register requests for the SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "Logging/LogSink.h"
#include "NetworkServices/SIP/Handlers/SipRegisterHandler.h"
#include "NetworkServices/Entities/NullEntity.h"
#include <random>

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipRegisterHandler, CSipHandler );
    IASLIB_REGISTER_SIP_HANDLER( CSipRegisterHandler );


    CSipRegisterHandler::CSipRegisterHandler( CString uri, CString version ) : CSipHandler( uri, version )
    {

    }

    CSipRegisterHandler::~CSipRegisterHandler( void )
    {

    }

    bool CSipRegisterHandler::process( CSipRequest *request, CSipResponse *response )
    {
        INFO_LOG( (const char *)request->toString() );

        response->addHeader( "From", request->getHeaderValue( "From" ) );
        response->addHeader( "To", request->getHeaderValue( "To" ) );
        response->addHeader( "Call-ID", request->getHeaderValue( "Call-ID" ) );
        response->addHeader( "CSeq", request->getHeaderValue( "CSeq" ) );
        response->addHeaders( "Via", request->getHeaderValues( "Via" ) );

        CString nonced;
        nonced.Format( "Digest realm=\"Payfone Fonebook SIP\",domain=\"sip:fonebook.org\", nonce=\"%s\",opaque=\"\", stale=FALSE, algorithm=MD5", (const char *)generateNonce() );
        response->addHeader( "WWW-Authenticate", (const char *)nonced  );

        // For now, we don't support registrations, so we're just going to 
        // reject them all with unauthorized.
        response->setStatus( 401, "Unauthorized" );

        CEntity *entity = new CNullEntity();

        // Set entity
        response->SetEntity( entity );

        return true;
    }

    CString CSipRegisterHandler::generateNonce( void )
    {
        const char *hex = "0123456789abcdef";
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distro(0,15);
        CString strRetVal = "";
        for ( int nX = 0; nX < 32; nX++ )
        {
            strRetVal += hex[distro(generator)];
        }
        
        return strRetVal;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
