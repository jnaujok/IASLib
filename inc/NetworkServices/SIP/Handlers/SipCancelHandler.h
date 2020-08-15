/**
 * SIP Cancel Handler class
 *
 * This class handles Cancel requests for the SIP server.
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

    class CSipCancelHandler : public CSipHandler
    {
        IASLIB_SET_SIP_HANDLER( CSipCancelHandler, "CANCEL" );
        public:
            DEFINE_OBJECT( CSipCancelHandler );

            CSipCancelHandler( CString uri, CString version );

            virtual ~CSipCancelHandler( void );

            virtual bool process( CSipRequest *request, CSipResponse *response );
    };

}
#endif // IASLIB_SIPCANCELHANDLER_H__
#endif // IASLIB_NETWORKING__

