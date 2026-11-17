/**
 * SIP Handler Factory class
 *
 * Holds handler prototypes by method and creates a handler for each
 * request. A server consults its factories in turn, so an application can
 * add a factory with its own handlers in front of the base factory to
 * replace or add methods.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 22, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPHANDLERFACTORY_H__
#define IASLIB_SIPHANDLERFACTORY_H__

#include "NetworkServices/SIP/Handlers/SipHandler.h"
#include "NetworkServices/SIP/SipRequest.h"
#include "NetworkServices/SIP/SipResponse.h"
#include "Collections/Array.h"
#include "Collections/StringArray.h"
#include "BaseTypes/Uuid.h"

namespace IASLib
{
    class CSipHandlerFactory : public CObject
    {
        protected:
            CArray          m_aHandlers;    // Prototypes, owned, in registration order.

        public:
            DEFINE_OBJECT( CSipHandlerFactory );

            CSipHandlerFactory( );

            virtual ~CSipHandlerFactory( void );

                // Creates a handler for the request's method, or returns NULL if
                // this factory has none. The caller owns the result.
            virtual CSipHandler *getHandler( CSipRequest *request, CUUID erid );

                // Takes ownership of the prototype. A later registration for the
                // same method replaces the earlier one.
            virtual void registerHandler( CSipHandler *handlerPrototype );

                // SIP methods are case-sensitive (RFC 3261 7.1).
            virtual bool isValidHandler( const char *method );

                // The methods this factory handles, in registration order.
            virtual CStringArray getMethods( void );

                // The methods as a comma-separated list, for an Allow header.
            virtual CString getAllowedList( void );
    };
} // namespace IASLib

#endif // IASLIB_SIPHANDLERFACTORY_H__

#endif // IASLIB_NETWORKING__
