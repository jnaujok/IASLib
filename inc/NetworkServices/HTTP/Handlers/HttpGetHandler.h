/**
 * @file HttpGetHandler.h
 * @brief The default GET handler (CHttpGetHandler).
 *
 * HTTP Get Handler class
 *
 * This class handles GET requests for the HTTP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: October 2, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPGETHANDLER_H__
#define IASLIB_HTTPGETHANDLER_H__

#include "NetworkServices/HTTP/Handlers/HttpHandler.h"

namespace IASLib
{
    /// The default GET handler.
    ///
    /// Answers 404 Not Found: there is nothing to serve.
    class CHttpGetHandler : public CHttpHandler
    {
        IASLIB_DEFINE_HTTP_HANDLER( CHttpGetHandler, "GET" );
        public:
            DEFINE_OBJECT( CHttpGetHandler );

                /// Creates a handler for one request (or, with empty values, a
                /// prototype to register).
            CHttpGetHandler( CString uri, CString version );

                /// Destroys the handler.
            virtual ~CHttpGetHandler( void );

                /// Answers 404 Not Found: there is nothing to serve.
                /// @return true.
            virtual bool process( CHttpRequest *request, CHttpResponse *response );
    };
}
#endif // IASLIB_HTTPGETHANDLER_H__

#endif // IASLIB_NETWORKING__
