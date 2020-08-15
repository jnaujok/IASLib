/**
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
    class CHttpHeadHandler : public CHttpHandler
    {
        IASLIB_DEFINE_HTTP_HANDLER( CHttpHeadHandler, "HEAD" );
        public:
            DEFINE_OBJECT( CHttpHeadHandler );

            CHttpHeadHandler( CString uri, CString version );

            virtual ~CHttpHeadHandler( void );

            virtual bool process( CHttpRequest *request, CHttpResponse *response );
    };
}
#endif // IASLIB_HTTPHEADHANDLER_H__

#endif // IASLIB_NETWORKING__
