/**
 * HTTP Get Handler class
 *
 * The default answer is 404 Not Found: there is nothing to serve.
 *
 * Author: Jeffrey R. Naujok
 * Created: October 4, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/Handlers/HttpGetHandler.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHttpGetHandler, CHttpHandler );

    CHttpGetHandler::CHttpGetHandler( CString uri, CString version ) : CHttpHandler( uri, version )
    {
    }

    CHttpGetHandler::~CHttpGetHandler( void )
    {
    }

    bool CHttpGetHandler::process( CHttpRequest *request, CHttpResponse *response )
    {
        response->setStatus( 404 );
        return true;
    }
}

#endif // IASLIB_NETWORKING__
