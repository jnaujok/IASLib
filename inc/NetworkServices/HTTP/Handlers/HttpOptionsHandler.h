/**
 * @file HttpOptionsHandler.h
 * @brief The default OPTIONS handler (CHttpOptionsHandler).
 *
 * HTTP Options Handler class
 *
 * This class handles OPTIONS requests for the HTTP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 29, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPOPTIONSHANDLER_H__
#define IASLIB_HTTPOPTIONSHANDLER_H__

#include "NetworkServices/HTTP/Handlers/HttpHandler.h"

namespace IASLib
{
    /// The default OPTIONS handler.
    ///
    /// Answers 200 OK with an Allow header listing the methods of the server's own factories.
    class CHttpOptionsHandler : public CHttpHandler
    {
        IASLIB_DEFINE_HTTP_HANDLER( CHttpOptionsHandler, "OPTIONS" );
        public:
            DEFINE_OBJECT( CHttpOptionsHandler );

                /// Creates a handler for one request (or, with empty values, a
                /// prototype to register).
            CHttpOptionsHandler( CString uri, CString version );

                /// Destroys the handler.
            virtual ~CHttpOptionsHandler( void );

                /// Answers 200 OK with an Allow header listing the methods of the server's own factories.
                /// @return true.
            virtual bool process( CHttpRequest *request, CHttpResponse *response );
    };
}
#endif // IASLIB_HTTPOPTIONSHANDLER_H__

#endif // IASLIB_NETWORKING__
