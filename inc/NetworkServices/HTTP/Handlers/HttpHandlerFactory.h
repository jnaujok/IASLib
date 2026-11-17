/**
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
    class CHttpHandlerFactory : public CObject
    {
        protected:
            CArray          m_aHandlers;    // Prototypes, owned, in registration order.

        public:
            DEFINE_OBJECT( CHttpHandlerFactory );

            CHttpHandlerFactory( );

            virtual ~CHttpHandlerFactory( void );

                // Creates a handler for the request's method, or NULL. The caller
                // owns the result.
            virtual CHttpHandler *getHandler( CHttpRequest *request, CUUID erid );
                // Creates a handler for the given method instead of the
                // request's (used to answer HEAD with the GET handler).
            virtual CHttpHandler *getHandler( const CString &method, CHttpRequest *request, CUUID erid );

                // Takes ownership of the prototype. A later registration for the
                // same method replaces the earlier one.
            virtual void registerHandler( CHttpHandler *handlerPrototype );

                // HTTP methods are case-sensitive (RFC 9110 9.1).
            virtual bool isValidHandler( const char *method );

            virtual CStringArray getMethods( void );
            virtual CString getAllowedList( void );
    };
} // namespace IASLib

#endif // IASLIB_HTTPHANDLERFACTORY_H__

#endif // IASLIB_NETWORKING__
