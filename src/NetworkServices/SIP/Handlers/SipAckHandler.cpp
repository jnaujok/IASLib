/**
 * SIP Ack Handler class
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/Handlers/SipAckHandler.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipAckHandler, CSipHandler );

    CSipAckHandler::CSipAckHandler( CString uri, CString version ) : CSipHandler( uri, version )
    {
    }

    CSipAckHandler::~CSipAckHandler( void )
    {
    }

    bool CSipAckHandler::process( CSipRequest *request, CSipResponse *response )
    {
        // Nothing to do: ACK has no response. A dialog-aware handler would
        // mark the session as established here.
        return true;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
