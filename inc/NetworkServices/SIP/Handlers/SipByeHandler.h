/**
 * @file SipByeHandler.h
 * @brief The default BYE handler (CSipByeHandler).
 *
 * SIP Bye Handler class
 *
 * This server keeps no dialogs, so a BYE cannot match one and is
 * answered with 481 Call/Transaction Does Not Exist (RFC 3261 15.1.2).
 * Applications that accept calls replace this handler.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPBYEHANDLER_H__
#define IASLIB_SIPBYEHANDLER_H__

#include "NetworkServices/SIP/Handlers/SipHandler.h"

namespace IASLib
{
    /// The default BYE handler.
    ///
    /// Answers 481 Call/Transaction Does Not Exist: this server keeps no dialogs (RFC 3261 15.1.2).
    class CSipByeHandler : public CSipHandler
    {
        IASLIB_SET_SIP_HANDLER( CSipByeHandler, "BYE" );
        public:
            DEFINE_OBJECT( CSipByeHandler );

                /// Creates a handler for one request (or, with empty values, a
                /// prototype to register).
            CSipByeHandler( CString uri, CString version );

                /// Destroys the handler.
            virtual ~CSipByeHandler( void );

                /// Answers 481 Call/Transaction Does Not Exist: this server keeps no dialogs (RFC 3261 15.1.2).
                /// @return true.
            virtual bool process( CSipRequest *request, CSipResponse *response );
    };
} // namespace IASLib

#endif // IASLIB_SIPBYEHANDLER_H__

#endif // IASLIB_NETWORKING__
