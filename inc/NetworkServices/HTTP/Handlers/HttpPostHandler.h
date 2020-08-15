/**
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
    class CHttpPostHandler : public CHttpHandler
    {
        IASLIB_DEFINE_HTTP_HANDLER( CHttpPostHandler, "POST" );
        public:
            DEFINE_OBJECT( CHttpPostHandler );

            CHttpPostHandler( CString uri, CString version );

            virtual ~CHttpPostHandler( void );

            virtual bool process( CHttpRequest *request, CHttpResponse *response );
    };
}
#endif // IASLIB_HTTPPOSTHANDLER_H__

#endif // IASLIB_NETWORKING__
