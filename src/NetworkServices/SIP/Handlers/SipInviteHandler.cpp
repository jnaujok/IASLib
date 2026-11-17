/**
 * SIP Invite Handler class
 *
 * Author: Jeffrey R. Naujok
 * Created: May 23, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/Handlers/SipInviteHandler.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipInviteHandler, CSipHandler );

    CSipInviteHandler::CSipInviteHandler( CString uri, CString version ) : CSipHandler( uri, version )
    {
    }

    CSipInviteHandler::~CSipInviteHandler( void )
    {
    }

    bool CSipInviteHandler::process( CSipRequest *request, CSipResponse *response )
    {
        response->setStatus( 404 );
        return true;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
