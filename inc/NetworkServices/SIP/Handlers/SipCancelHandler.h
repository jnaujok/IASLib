/**
 * @file SipCancelHandler.h
 * @brief The default CANCEL handler (CSipCancelHandler).
 *
 * SIP Cancel Handler class
 *
 * A CANCEL that matches a pending INVITE is handled by the server's
 * transaction layer, which answers it with 200 and ends the INVITE with
 * 487. This handler is only called for a CANCEL that matches no
 * transaction, which RFC 3261 9.2 answers with 481.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPCANCELHANDLER_H__
#define IASLIB_SIPCANCELHANDLER_H__

#include "NetworkServices/SIP/Handlers/SipHandler.h"

namespace IASLib
{
    /// The default CANCEL handler.
    ///
    /// Answers 481. It is called only for a CANCEL that matches no transaction (RFC 3261 9.2); the server answers one that matches a pending INVITE itself.
    class CSipCancelHandler : public CSipHandler
    {
        IASLIB_SET_SIP_HANDLER( CSipCancelHandler, "CANCEL" );
        public:
            DEFINE_OBJECT( CSipCancelHandler );

                /// Creates a handler for one request (or, with empty values, a
                /// prototype to register).
            CSipCancelHandler( CString uri, CString version );

                /// Destroys the handler.
            virtual ~CSipCancelHandler( void );

                /// Answers 481. It is called only for a CANCEL that matches no transaction (RFC 3261 9.2); the server answers one that matches a pending INVITE itself.
                /// @return true.
            virtual bool process( CSipRequest *request, CSipResponse *response );
    };
} // namespace IASLib

#endif // IASLIB_SIPCANCELHANDLER_H__

#endif // IASLIB_NETWORKING__
