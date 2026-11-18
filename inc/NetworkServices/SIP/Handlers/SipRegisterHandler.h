/**
 * @file SipRegisterHandler.h
 * @brief The default REGISTER handler (CSipRegisterHandler).
 *
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
    /// The default REGISTER handler.
    ///
    /// Answers a REGISTER without credentials with 401 and a Digest challenge (the server's realm and a fresh nonce), and one with an Authorization header with 403, as there are no users to check against.
    class CSipRegisterHandler : public CSipHandler
    {
        IASLIB_SET_SIP_HANDLER( CSipRegisterHandler, "REGISTER" );
        public:
            DEFINE_OBJECT( CSipRegisterHandler );

                /// Creates a handler for one request (or, with empty values, a
                /// prototype to register).
            CSipRegisterHandler( CString uri, CString version );

                /// Destroys the handler.
            virtual ~CSipRegisterHandler( void );

                /// Answers a REGISTER without credentials with 401 and a Digest challenge (the server's realm and a fresh nonce), and one with an Authorization header with 403, as there are no users to check against.
                /// @return true.
            virtual bool process( CSipRequest *request, CSipResponse *response );

                /// A random 128-bit nonce, as 32 hexadecimal digits.
            static CString generateNonce( void );
    };
} // namespace IASLib

#endif // IASLIB_SIPREGISTERHANDLER_H__
#endif // IASLIB_NETWORKING__
