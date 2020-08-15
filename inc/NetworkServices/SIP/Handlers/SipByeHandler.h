/**
 * SIP Bye Handler class
 *
 * This class handles Ack requests for the SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPBYEHANDLER_H__
#define IASLIB_SIPBYEHANDLER_H__

#include "NetworkServices/SIP/Handlers/SipHandler.h"

namespace IASLib
{
    class CSipByeHandler : public CSipHandler
    {
        IASLIB_SET_SIP_HANDLER( CSipByeHandler, "BYE" );
        public:
            DEFINE_OBJECT( CSipByeHandler );

            CSipByeHandler( CString uri, CString version );

            virtual ~CSipByeHandler( void );

            virtual bool process( CSipRequest *request, CSipResponse *response );
    };
} // namespace IASLib

#endif // IASLIB_SIPBYEHANDLER_H__
#endif // IASLIB_NETWORKING__

