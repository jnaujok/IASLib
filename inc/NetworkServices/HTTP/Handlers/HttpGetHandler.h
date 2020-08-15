/**
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
    class CHttpGetHandler : public CHttpHandler
    {
        IASLIB_DEFINE_HTTP_HANDLER( CHttpGetHandler, "GET" );
        public:
            DEFINE_OBJECT( CHttpGetHandler );

            CHttpGetHandler( CString uri, CString version );

            virtual ~CHttpGetHandler( void );

            virtual bool process( CHttpRequest *request, CHttpResponse *response );
    };
}
#endif // IASLIB_HTTPGETHANDLER_H__

#endif // IASLIB_NETWORKING__
