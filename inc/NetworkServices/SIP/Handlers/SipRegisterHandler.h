/**
 * SIP Register Handler class
 *
 * This server has no registrar or credential store, so the default
 * REGISTER handler challenges a REGISTER that carries no credentials with
 * 401 Unauthorized and a Digest challenge (a fresh random nonce each time),
 * and rejects one that carries credentials with 403 Forbidden, so clients
 * do not retry forever. Applications that act as a registrar replace this
 * handler and check the Authorization header against their own users.
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

                // A random 128-bit nonce, as 32 hexadecimal digits.
            static CString generateNonce( void );
    };
} // namespace IASLib

#endif // IASLIB_SIPREGISTERHANDLER_H__
#endif // IASLIB_NETWORKING__
