/**
 * SIP Bye Handler class
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/Handlers/SipByeHandler.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipByeHandler, CSipHandler );

    CSipByeHandler::CSipByeHandler( CString uri, CString version ) : CSipHandler( uri, version )
    {
    }

    CSipByeHandler::~CSipByeHandler( void )
    {
    }

    bool CSipByeHandler::process( CSipRequest *request, CSipResponse *response )
    {
        response->setStatus( 481 );
        return true;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
