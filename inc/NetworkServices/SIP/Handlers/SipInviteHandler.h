/**
 * SIP Invite Handler class
 *
 * This class handles Invite requests for the SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 23, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPINVITEHANDLER_H__
#define IASLIB_SIPINVITEHANDLER_H__

#include "NetworkServices/SIP/Handlers/SipHandler.h"
#include "BaseTypes/String_.h"

namespace IASLib
{
    class CSipInviteHandler : public CSipHandler
    {
        IASLIB_SET_SIP_HANDLER( CSipInviteHandler, "INVITE" );
        public:
            DEFINE_OBJECT( CSipInviteHandler );

            CSipInviteHandler( CString uri, CString version );

            virtual ~CSipInviteHandler( void );

            virtual bool process( CSipRequest *request, CSipResponse *response );
        private:
            CString extractPhoneNumber( const char *uriString );
    };
} // namespace IASLib

#endif // IASLIB_SIPINVITEHANDLER_H__

#endif // IASLIB_NETWORKING__

