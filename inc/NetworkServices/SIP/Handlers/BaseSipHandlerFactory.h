/**
 * @file BaseSipHandlerFactory.h
 * @brief The default SIP handlers (CBaseSipHandlerFactory).
 *
 * Base SIP Handler Factory class
 *
 * The default handlers: INVITE, ACK, BYE, CANCEL, OPTIONS and REGISTER.
 * They describe a user agent that exists but has no users and no calls;
 * see each handler for its response. Applications replace these by adding
 * their own factory in front of this one, or by registering their own
 * handler for the same method here.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 22, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_BASESIPHANDLERFACTORY_H__
#define IASLIB_BASESIPHANDLERFACTORY_H__

#include "SipHandlerFactory.h"

namespace IASLib
{
    /// A factory with the default handlers for INVITE, ACK, BYE, CANCEL,
    /// OPTIONS and REGISTER.
    ///
    /// They describe a user agent that exists but has no users and no calls:
    /// INVITE gets 404, BYE and unmatched CANCEL 481, OPTIONS 200 with what
    /// the server supports, and REGISTER a Digest challenge (401) or 403.
    /// Applications replace these by adding their own factory after this one
    /// (so it is consulted first), or by registering their own handler for
    /// the same method here.
    class CBaseSipHandlerFactory : public CSipHandlerFactory
    {
        public:
                /// Registers the default handlers.
            CBaseSipHandlerFactory( void );
                /// Deletes the handlers.
            virtual ~CBaseSipHandlerFactory( void );

            DEFINE_OBJECT( CBaseSipHandlerFactory );
    };
} // namespace IASLib

#endif // IASLIB_BASESIPHANDLERFACTORY_H__

#endif // IASLIB_NETWORKING__
