/**
 * @file SipOptionsHandler.h
 * @brief The default OPTIONS handler (CSipOptionsHandler).
 *
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
    /// The default OPTIONS handler.
    ///
    /// Answers 200 OK with Allow and (if any) Supported from the server, and Accept (application/sdp), Accept-Encoding and Accept-Language headers.
    class CSipOptionsHandler : public CSipHandler
    {
        IASLIB_SET_SIP_HANDLER( CSipOptionsHandler, "OPTIONS" );
        public:
            DEFINE_OBJECT( CSipOptionsHandler );

                /// Creates a handler for one request (or, with empty values, a
                /// prototype to register).
            CSipOptionsHandler( CString uri, CString version );

                /// Destroys the handler.
            virtual ~CSipOptionsHandler( void );

                /// Answers 200 OK with Allow and (if any) Supported from the server, and Accept (application/sdp), Accept-Encoding and Accept-Language headers.
                /// @return true.
            virtual bool process( CSipRequest *request, CSipResponse *response );
    };
} // namespace IASLib

#endif // IASLIB_SIPOPTIONSHANDLER_H__

#endif // IASLIB_NETWORKING__
