/**
 * @file SipInviteHandler.h
 * @brief The default INVITE handler (CSipInviteHandler).
 *
 * SIP Invite Handler class
 *
 * The default answer to an INVITE is 404 Not Found: this user agent has
 * no users. Applications replace this handler to accept or reject calls.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 23, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPINVITEHANDLER_H__
#define IASLIB_SIPINVITEHANDLER_H__

#include "NetworkServices/SIP/Handlers/SipHandler.h"

namespace IASLib
{
    /// The default INVITE handler.
    ///
    /// Answers 404 Not Found: this user agent has no users.
    class CSipInviteHandler : public CSipHandler
    {
        IASLIB_SET_SIP_HANDLER( CSipInviteHandler, "INVITE" );
        public:
            DEFINE_OBJECT( CSipInviteHandler );

                /// Creates a handler for one request (or, with empty values, a
                /// prototype to register).
            CSipInviteHandler( CString uri, CString version );

                /// Destroys the handler.
            virtual ~CSipInviteHandler( void );

                /// Answers 404 Not Found: this user agent has no users.
                /// @return true.
            virtual bool process( CSipRequest *request, CSipResponse *response );
    };
} // namespace IASLib

#endif // IASLIB_SIPINVITEHANDLER_H__

#endif // IASLIB_NETWORKING__
