/**
 * SIP Cancel Handler class
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/Handlers/SipCancelHandler.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipCancelHandler, CSipHandler );

    CSipCancelHandler::CSipCancelHandler( CString uri, CString version ) : CSipHandler( uri, version )
    {
    }

    CSipCancelHandler::~CSipCancelHandler( void )
    {
    }

    bool CSipCancelHandler::process( CSipRequest *request, CSipResponse *response )
    {
        response->setStatus( 481 );
        return true;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
