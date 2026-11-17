/**
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
    class CBaseSipHandlerFactory : public CSipHandlerFactory
    {
        public:
            CBaseSipHandlerFactory( void );
            virtual ~CBaseSipHandlerFactory( void );

            DEFINE_OBJECT( CBaseSipHandlerFactory );
    };
} // namespace IASLib

#endif // IASLIB_BASESIPHANDLERFACTORY_H__

#endif // IASLIB_NETWORKING__
