/**
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
    class CHttpOptionsHandler : public CHttpHandler
    {
        IASLIB_DEFINE_HTTP_HANDLER( CHttpOptionsHandler, "OPTIONS" );
        public:
            DEFINE_OBJECT( CHttpOptionsHandler );

            CHttpOptionsHandler( CString uri, CString version );

            virtual ~CHttpOptionsHandler( void );

            virtual bool process( CHttpRequest *request, CHttpResponse *response );
    };
}
#endif // IASLIB_HTTPOPTIONSHANDLER_H__

#endif // IASLIB_NETWORKING__
