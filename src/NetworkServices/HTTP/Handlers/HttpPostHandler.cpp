/**
 * HTTP Post Handler class
 *
 * The default answer is 404 Not Found: there is nothing to serve.
 *
 * Author: Jeffrey R. Naujok
 * Created: October 4, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/Handlers/HttpPostHandler.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHttpPostHandler, CHttpHandler );

    CHttpPostHandler::CHttpPostHandler( CString uri, CString version ) : CHttpHandler( uri, version )
    {
    }

    CHttpPostHandler::~CHttpPostHandler( void )
    {
    }

    bool CHttpPostHandler::process( CHttpRequest *request, CHttpResponse *response )
    {
        response->setStatus( 404 );
        return true;
    }
}

#endif // IASLIB_NETWORKING__
