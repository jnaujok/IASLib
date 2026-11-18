/**
 * @file HttpHandlerFactory.h
 * @brief A set of HTTP handler prototypes, one per method (CHttpHandlerFactory).
 *
 * HTTP Handler Factory class
 *
 * Holds handler prototypes by method and creates a handler for each
 * request. A server consults its factories in turn, newest first.
 *
 * Author: Jeffrey R. Naujok
 * Created: October 4, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPHANDLERFACTORY_H__
#define IASLIB_HTTPHANDLERFACTORY_H__

#include "NetworkServices/HTTP/Handlers/HttpHandler.h"
#include "NetworkServices/HTTP/HttpRequest.h"
#include "NetworkServices/HTTP/HttpResponse.h"
#include "Collections/Array.h"
#include "Collections/StringArray.h"
#include "BaseTypes/Uuid.h"

namespace IASLib
{
    /// Holds handler prototypes by method and creates a handler for each request.
    ///
    /// A server consults its factories in turn, newest first, and uses the
    /// first handler offered. Derived factories normally register their
    /// prototypes in their constructor. The factory owns its prototypes.
    /// Register prototypes before the server starts; getHandler() may then be
    /// called from several threads at once.
    class CHttpHandlerFactory : public CObject
    {
        protected:
            CArray          m_aHandlers;    ///< Prototypes, owned, in registration order.

        public:
            DEFINE_OBJECT( CHttpHandlerFactory );

                /// Creates a factory with no handlers.
            CHttpHandlerFactory( );

                /// Deletes the prototypes.
            virtual ~CHttpHandlerFactory( void );

                /// Creates a handler for the request's method, or NULL.
                /// @return A new handler with its id set, owned by the caller.
            virtual CHttpHandler *getHandler( CHttpRequest *request, CUUID erid );
                /// Creates a handler for the given method instead of the
                /// request's (used to answer HEAD with the GET handler).
                /// @return A new handler with its id set, owned by the caller, or
                ///         NULL if no prototype handles the method.
            virtual CHttpHandler *getHandler( const CString &method, CHttpRequest *request, CUUID erid );

                /// Takes ownership of the prototype. A later registration for the
                /// same method replaces (and deletes) the earlier one. NULL is
                /// ignored.
            virtual void registerHandler( CHttpHandler *handlerPrototype );

                /// Whether a prototype handles the method. HTTP methods are
                /// case-sensitive (RFC 9110 9.1).
            virtual bool isValidHandler( const char *method );

                /// The methods handled, in registration order.
            virtual CStringArray getMethods( void );
                /// The methods handled, comma-separated, for an Allow header.
            virtual CString getAllowedList( void );
    };
} // namespace IASLib

#endif // IASLIB_HTTPHANDLERFACTORY_H__

#endif // IASLIB_NETWORKING__
