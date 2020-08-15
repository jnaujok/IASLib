/**
 * SIP Register Handler class
 *
 * This class handles Register requests for the SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPREGISTERHANDLER_H__
#define IASLIB_SIPREGISTERHANDLER_H__

#include "NetworkServices/SIP/Handlers/SipHandler.h"

namespace IASLib
{
    class CSipRegisterHandler : public CSipHandler
    {
        IASLIB_SET_SIP_HANDLER( CSipRegisterHandler, "REGISTER" );
        public:
            DEFINE_OBJECT( CSipRegisterHandler );

            CSipRegisterHandler( CString uri, CString version );

            virtual ~CSipRegisterHandler( void );

            virtual bool process( CSipRequest *request, CSipResponse *response );

        private:
            CString generateNonce( void );
    };
} // namespace IASLib

#endif // IASLIB_SIPREGISTERHANDLER_H__
#endif // IASLIB_NETWORKING__
