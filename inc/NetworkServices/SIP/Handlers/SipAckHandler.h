/**
 * @file SipAckHandler.h
 * @brief The default ACK handler (CSipAckHandler).
 *
 * SIP Ack Handler class
 *
 * This handler is called for an ACK that the server's transaction layer
 * did not absorb, that is, an ACK for a 2xx response (which belongs to the
 * dialog, not the INVITE transaction) or one that matches nothing. ACK is
 * never answered (RFC 3261 17.1.1.3), so this default does nothing.
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
    /// The default ACK handler.
    ///
    /// Does nothing: ACK is never answered (RFC 3261 17.1.1.3). It is called only for an ACK the transaction layer did not absorb, such as the ACK for a 2xx response.
    class CSipAckHandler : public CSipHandler
    {
        IASLIB_SET_SIP_HANDLER( CSipAckHandler, "ACK" );
        public:
            DEFINE_OBJECT( CSipAckHandler );

                /// Creates a handler for one request (or, with empty values, a
                /// prototype to register).
            CSipAckHandler( CString uri, CString version );

                /// Destroys the handler.
            virtual ~CSipAckHandler( void );

                /// Does nothing: ACK is never answered (RFC 3261 17.1.1.3). It is called only for an ACK the transaction layer did not absorb, such as the ACK for a 2xx response.
                /// @return true.
            virtual bool process( CSipRequest *request, CSipResponse *response );
    };
} // namespace IASLib

#endif // IASLIB_SIPACKHANDLER_H__

#endif // IASLIB_NETWORKING__
