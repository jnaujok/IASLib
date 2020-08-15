/**
 * HTTP Handler Factory class
 *
 * This class provides a factory for generating HTTP Handlers. 
 * 
 * Factories have handlers registered to them, and then they are added
 * to the HTTP Server. Once added to the server, they override any
 * factories previously added, allowing a chain of handlers to deal 
 * with any particular request.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 29, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPHANDLERFACTORY_H__
#define IASLIB_HTTPHANDLERFACTORY_H__

#include "NetworkServices/HTTP/Handlers/HttpHandler.h"
#include "NetworkServices/HTTP/HttpRequest.h"
#include "NetworkServices/HTTP/HttpResponse.h"
#include "BaseTypes/Uuid.h"

namespace IASLib
{
    class CHttpHandlerFactory : public CObject
    {
        protected:
            CHash           m_handlerMap;

        public:
            DEFINE_OBJECT( CHttpHandlerFactory );

            CHttpHandlerFactory( );

            virtual ~CHttpHandlerFactory( void );

            virtual CHttpHandler *getHandler( CHttpRequest *request, CUUID erid );
            virtual void registerHandler( CHttpHandler *handlerFactory );
            virtual bool isValidHandler( const char *method ); 

            static CString getAllowedList( void );
    };

} // namespace IASLib

#endif // IASLIB_HTTPHANDLERFACTORY_H__

#endif // IASLIB_NETWORKING__

