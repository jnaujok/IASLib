/**
 * @file HttpPostHandler.h
 * @brief The default POST handler (CHttpPostHandler).
 *
 * HTTP Post Handler class
 *
 * This class handles POST requests for the HTTP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 28, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPPOSTHANDLER_H__
#define IASLIB_HTTPPOSTHANDLER_H__

#include "NetworkServices/HTTP/Handlers/HttpHandler.h"

namespace IASLib
{
    /// The default POST handler.
    ///
    /// Answers 404 Not Found: there is nothing to serve.
    class CHttpPostHandler : public CHttpHandler
    {
        IASLIB_DEFINE_HTTP_HANDLER( CHttpPostHandler, "POST" );
        public:
            DEFINE_OBJECT( CHttpPostHandler );

                /// Creates a handler for one request (or, with empty values, a
                /// prototype to register).
            CHttpPostHandler( CString uri, CString version );

                /// Destroys the handler.
            virtual ~CHttpPostHandler( void );

                /// Answers 404 Not Found: there is nothing to serve.
                /// @return true.
            virtual bool process( CHttpRequest *request, CHttpResponse *response );
    };
}
#endif // IASLIB_HTTPPOSTHANDLER_H__

#endif // IASLIB_NETWORKING__
