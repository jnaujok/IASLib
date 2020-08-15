/**
 * SIP Options Handler class
 *
 * This class handles Options requests for the SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 13, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPOPTIONSHANDLER_H__
#define IASLIB_SIPOPTIONSHANDLER_H__

#include "NetworkServices/SIP/Handlers/SipHandler.h"

namespace IASLib
{
    class CSipOptionsHandler : public CSipHandler
    {
        IASLIB_SET_SIP_HANDLER( CSipOptionsHandler, "OPTIONS" );
        public:
            DEFINE_OBJECT( CSipOptionsHandler );

            CSipOptionsHandler( CString uri, CString version );

            virtual ~CSipOptionsHandler( void );

            virtual bool process( CSipRequest *request, CSipResponse *response );
    };
} // namespace IASLib

#endif // IASLIB_SIPOPTIONSHANDLER_H__

#endif // IASLIB_NETWORKING__
