/**
 * SIP Options Handler class
 *
 * Answers OPTIONS (RFC 3261 section 11) with 200 OK and this server's
 * capabilities: Allow (the methods of all of the server's handlers),
 * Accept, Accept-Encoding, Accept-Language and, if the server supports any
 * extensions, Supported. No session description is included.
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
