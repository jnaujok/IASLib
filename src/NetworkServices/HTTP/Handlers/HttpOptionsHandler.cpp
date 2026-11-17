/**
 * HTTP Options Handler class
 *
 * Answers OPTIONS with 200 OK and an Allow header listing the methods of
 * all of the server's handlers (RFC 9110 9.3.7).
 *
 * Author: Jeffrey R. Naujok
 * Created: October 4, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/Handlers/HttpOptionsHandler.h"
#include "NetworkServices/HTTP/HttpServer.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHttpOptionsHandler, CHttpHandler );

    CHttpOptionsHandler::CHttpOptionsHandler( CString uri, CString version ) : CHttpHandler( uri, version )
    {
    }

    CHttpOptionsHandler::~CHttpOptionsHandler( void )
    {
    }

    bool CHttpOptionsHandler::process( CHttpRequest *request, CHttpResponse *response )
    {
        response->setStatus( 200 );
        if ( m_pServer )
        {
            response->addHeader( "Allow", m_pServer->getAllowedList() );
        }
        return true;
    }
}

#endif // IASLIB_NETWORKING__
