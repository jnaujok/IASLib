/**
 * @file HttpHeadHandler.h
 * @brief A HEAD handler (CHttpHeadHandler).
 *
 * HTTP HEAD Handler class
 *
 * This class handles HEAD requests for the HTTP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 28, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPHEADHANDLER_H__
#define IASLIB_HTTPHEADHANDLER_H__

#include "NetworkServices/HTTP/Handlers/HttpHandler.h"

namespace IASLib
{
    /// A HEAD handler.
    ///
    /// Answers 404 Not Found. CBaseHttpHandlerFactory does not register it, so that HEAD is answered by the GET handler.
    class CHttpHeadHandler : public CHttpHandler
    {
        IASLIB_DEFINE_HTTP_HANDLER( CHttpHeadHandler, "HEAD" );
        public:
            DEFINE_OBJECT( CHttpHeadHandler );

                /// Creates a handler for one request (or, with empty values, a
                /// prototype to register).
            CHttpHeadHandler( CString uri, CString version );

                /// Destroys the handler.
            virtual ~CHttpHeadHandler( void );

                /// Answers 404 Not Found. CBaseHttpHandlerFactory does not register it, so that HEAD is answered by the GET handler.
                /// @return true.
            virtual bool process( CHttpRequest *request, CHttpResponse *response );
    };
}
#endif // IASLIB_HTTPHEADHANDLER_H__

#endif // IASLIB_NETWORKING__
