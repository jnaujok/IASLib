/**
 * SIP Ack Handler class
 *
 * This class handles Ack requests for the SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPACKHANDLER_H__
#define IASLIB_SIPACKHANDLER_H__

#include "NetworkServices/SIP/Handlers/SipHandler.h"

namespace IASLib
{
    class CSipAckHandler : public CSipHandler
    {
        IASLIB_SET_SIP_HANDLER( CSipAckHandler, "ACK" );
        public:
            DEFINE_OBJECT( CSipAckHandler );

            CSipAckHandler( CString uri, CString version );

            virtual ~CSipAckHandler( void );

            virtual bool process( CSipRequest *request, CSipResponse *response );
    };
}
#endif // IASLIB_SIPACKHANDLER_H__

#endif // IASLIB_NETWORKING__
